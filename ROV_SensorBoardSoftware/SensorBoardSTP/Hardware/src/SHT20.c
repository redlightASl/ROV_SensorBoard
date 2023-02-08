#include "sht20_iic.h"
#include "SHT20.h"
#include "delay.h"
#include <stdio.h>
#include "MultiTimer.h"

SHT20_OriginData_t origin_data;
SHT20_Data_t calc_data;

// volatile MultiTimer SHT20_recv_timer;
MultiTimer SHT20_recv_timer;
uint8_t SHT20_STATE = SHT20_STATE_SEND_CMD1;

static inline void SHT20_SendCommand(uint8_t cmd)
{
    SHT20_IIC_Start();
    SHT20_IIC_Send_Byte((SHT2x_I2C_ADDR << 1) | 0);
    SHT20_IIC_Wait_Ack();
    SHT20_IIC_Send_Byte(cmd);
    SHT20_IIC_Wait_Ack();
    SHT20_IIC_Stop();
}

static inline uint16_t SHT20_RecvRawData(void)
{
    uint8_t val[3] = { 0 };
    SHT20_IIC_Start();
    SHT20_IIC_Send_Byte((SHT2x_I2C_ADDR << 1) | 1);
    SHT20_IIC_Wait_Ack();
    val[0] = SHT20_IIC_Read_Byte(1);
    val[1] = SHT20_IIC_Read_Byte(1);
    val[2] = SHT20_IIC_Read_Byte(1);
    SHT20_IIC_Stop();
    return (uint16_t)(((uint16_t)val[0] << 8) | val[1]);
}

// static uint16_t SHT20_GetRaw(uint8_t cmd)
// {
//     SHT20_SendCommand(cmd);
//     delay_ms(40);
//     return SHT20_RecvRawData();
// }

// static float SHT20_GetTemperature(uint8_t hold)
// {
//     uint8_t cmd = (hold ? SHT2x_READ_TEMP_HOLD : SHT2x_READ_TEMP_NOHOLD);
//     return -46.85 + 175.72 * (SHT20_GetRaw(cmd) / 65536.0);
// }

// static float SHT20_GetRelativeHumidity(uint8_t hold)
// {
//     uint8_t cmd = (hold ? SHT2x_READ_RH_HOLD : SHT2x_READ_RH_NOHOLD);
//     return -6 + 125.00 * (SHT20_GetRaw(cmd) / 65536.0);
// }

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

void SHT20_GetData(SHT20_Data_t* sht20_data)
{
    sht20_data->humidity = calc_data.humidity;
    sht20_data->temperture = calc_data.temperture;
}

float SHT20_CelsiusToKelvin(float celsius)
{
    return celsius + 273;
}

