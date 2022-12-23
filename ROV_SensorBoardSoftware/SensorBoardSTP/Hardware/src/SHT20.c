#include "sht20_iic.h"
#include "SHT20.h"
#include "delay.h"
#include <stdio.h>
#include "MultiTimer.h"

SHT20_OriginData_t origin_data;
SHT20_Data_t calc_data;

volatile MultiTimer SHT20_recv_timer;
u8 SHT20_STATE = SHT20_STATE_SEND_CMD1;

static inline void SHT20_SendCommand(u8 cmd)
{
    SHT20_IIC_Start();
    SHT20_IIC_Send_Byte((SHT2x_I2C_ADDR << 1) | 0);
    SHT20_IIC_Wait_Ack();
    SHT20_IIC_Send_Byte(cmd);
    SHT20_IIC_Wait_Ack();
    SHT20_IIC_Stop();
}

static inline u16 SHT20_RecvRawData(void)
{
    u8 val[3] = { 0 };
    SHT20_IIC_Start();
    SHT20_IIC_Send_Byte((SHT2x_I2C_ADDR << 1) | 1);
    SHT20_IIC_Wait_Ack();
    val[0] = SHT20_IIC_Read_Byte(1);
    val[1] = SHT20_IIC_Read_Byte(1);
    val[2] = SHT20_IIC_Read_Byte(1);
    SHT20_IIC_Stop();
    return (u16)(((u16)val[0] << 8) | val[1]);
}

static u16 SHT20_GetRaw(u8 cmd)
{
    SHT20_SendCommand(cmd);
    delay_ms(40);
    return SHT20_RecvRawData();
}

static float SHT20_GetTemperature(u8 hold)
{
    u8 cmd = (hold ? SHT2x_READ_TEMP_HOLD : SHT2x_READ_TEMP_NOHOLD);
    return -46.85 + 175.72 * (SHT20_GetRaw(cmd) / 65536.0);
}



static float SHT20_GetRelativeHumidity(u8 hold)
{
    u8 cmd = (hold ? SHT2x_READ_RH_HOLD : SHT2x_READ_RH_NOHOLD);
    return -6 + 125.00 * (SHT20_GetRaw(cmd) / 65536.0);
}

void SHT20_Init(void)
{
    while (!SHT20_SoftReset())
    {
        printf("SHT20 Init Failed\r\n");
        delay_ms(100);
    }
    printf("SHT20 Init Successful\r\n");

    delay_ms(200);

    while (!SHT20_SetResolution(RES_12_8))
    {
        printf("SHT20 Setup Failed\r\n");
        delay_ms(100);
    }
    printf("SHT20 Setup Successful\r\n");
}

// void SHT20_GetData(float* temp, float* rh)
// {
//     *temp = SHT20_GetTemperature(0);
//     *rh = SHT20_GetRelativeHumidity(0);
// }

void SHT20_getData(float* temp, float* rh)
{
    *temp = calc_data.temperture;
    *rh = calc_data.humidity;
}

void SHT20_GetDataTask_cb(MultiTimer* timer, void* userData)
{
    switch (SHT20_STATE)
    {
    case SHT20_STATE_SEND_CMD1:
        SHT20_SendCommand(SHT2x_READ_TEMP_NOHOLD);
        SHT20_STATE = SHT20_STATE_GET_TEMP;
        break;
    case SHT20_STATE_GET_TEMP:
        origin_data.RawTemperature = SHT20_RecvRawData();
        SHT20_STATE = SHT20_STATE_SEND_CMD2;
        break;
    case SHT20_STATE_SEND_CMD2:
        SHT20_SendCommand(SHT2x_READ_RH_NOHOLD);
        SHT20_STATE = SHT20_STATE_GET_HUMI;
        break;
    case SHT20_STATE_GET_HUMI:
        origin_data.RawRelativeHumidity = SHT20_RecvRawData();
        SHT20_STATE = SHT20_STATE_CALCULATE;
        break;
    case SHT20_STATE_CALCULATE:
        calc_data.temperture = -46.85 + 175.72 * (origin_data.RawTemperature / 65536.0);
        calc_data.humidity = -6 + 125.00 * (origin_data.RawRelativeHumidity / 65536.0);;
        SHT20_STATE = SHT20_STATE_SEND_CMD1;
        break;
    default:
        SHT20_STATE = SHT20_STATE_SEND_CMD1;
        break;
    }
    MultiTimerRestart(timer, (uint32_t)(userData));
}

float SHT20_CelsiusToKelvin(float celsius)
{
    return celsius + 273;
}

