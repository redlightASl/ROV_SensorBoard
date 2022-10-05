#include "sht20_iic.h"
#include "SHT20.h"
#include "delay.h"
#include <stdio.h>

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

void SHT20_GetData(float* temp, float* rh)
{
    *temp = SHT20_GetTemperature(0);
    *rh = SHT20_GetRelativeHumidity(0);
}

float SHT20_CelsiusToKelvin(float celsius)
{
	return celsius + 273;
}

