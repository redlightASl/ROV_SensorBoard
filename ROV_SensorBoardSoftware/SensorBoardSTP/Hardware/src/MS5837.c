#include <stdio.h>
#include "delay.h"
#include "myiic.h"
#include "MS5837.h"
#include "MultiTimer.h"

static MS5837_Attribute_t ms5832_attribute;
static MS5837_OriginData_t ms5837_origin_data;
static MS5837_Data_t ms5837_data;
static MS5837_FIR_Parameter_t ms5837_fir;

// volatile MultiTimer MS5837_recv_timer;
MultiTimer MS5837_recv_timer;
uint8_t MS5837_STATE = MS5837_STATE_CONVERT_D1;

static void MS5837_WriteByte(uint8_t WriteCmd)
{
	IIC_Start();
	IIC_Send_Byte((MS5837_ADDR << 1) | 0);
	IIC_Wait_Ack();
	IIC_Send_Byte(WriteCmd);
	IIC_Wait_Ack();
	IIC_Stop();
}

static uint32_t MS5837_Read4Byte(uint8_t ReadAddr)
{
	uint32_t res = 0;
	uint8_t data[4] = { 0, 0, 0, 0 };

	IIC_Start();
	IIC_Send_Byte((MS5837_ADDR << 1) | 0);
	IIC_Wait_Ack();
	IIC_Send_Byte(ReadAddr);
	IIC_Wait_Ack();
	IIC_Start();
	IIC_Send_Byte((MS5837_ADDR << 1) | 1);
	IIC_Wait_Ack();
	data[0] = IIC_Read_Byte(1);
	data[1] = IIC_Read_Byte(1);
	data[2] = IIC_Read_Byte(1);
	data[3] = IIC_Read_Byte(0);
	IIC_Stop();
	res = ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);
	return res;
}

static inline void MS5837_SetFluidDensity(float density)
{
	ms5832_attribute.fluidDensity = density;
}

static inline void MS5837_SetModel(uint8_t model)
{
	ms5832_attribute.model = model;
}

static inline void MS5837_SetUnit(float pressure_unit, float temperature_unit)
{
	ms5832_attribute.pressureUnit = pressure_unit;
	ms5832_attribute.tempertureUnit = temperature_unit;
}

static inline void MS5837_SetRefPressure(float reference_pressure)
{
	ms5832_attribute.reference_pressure = reference_pressure;
}

static inline void MS5837_Reset(void)
{
	IIC_Start();
	IIC_Send_Byte((MS5837_ADDR << 1) | 0);
	IIC_Wait_Ack();
	IIC_Send_Byte(MS5837_RESET);
	IIC_Wait_Ack();
	IIC_Stop();
}

static uint8_t crc4(uint16_t n_prom[])
{
	uint16_t n_rem = 0;
	uint8_t i = 0;
	uint8_t n_bit = 8;

	n_prom[0] = ((n_prom[0]) & 0x0FFF);
	n_prom[7] = 0;

	for (i = 0;i < 16; i++)
	{
		if (i % 2 == 1)
		{
			n_rem ^= (uint16_t)((n_prom[i >> 1]) & 0x00FF);
		}
		else
		{
			n_rem ^= (uint16_t)(n_prom[i >> 1] >> 8);
		}

		for (n_bit = 8;n_bit > 0;n_bit--)
		{
			if (n_rem & 0x8000)
			{
				n_rem = (n_rem << 1) ^ 0x3000;
			}
			else
			{
				n_rem = (n_rem << 1);
			}
		}
	}
	n_rem = ((n_rem >> 12) & 0x000F);

	return n_rem ^ 0x00;
}

static uint8_t MS5837_Check(void)
{
	uint8_t i = 0;
	uint8_t data[2] = { 0,0 };

	uint8_t crcRead = 0;
	uint8_t crcCalculated = 0;

	MS5837_Reset();
	delay_ms(20);

	for (i = 0; i < 7; i++)
	{
		IIC_Start();
		IIC_Send_Byte((MS5837_ADDR << 1) | 0);
		IIC_Wait_Ack();
		IIC_Send_Byte(MS5837_PROM_READ + (i * 2));
		IIC_Wait_Ack();
		IIC_Stop();
		delay_us(5);
		IIC_Start();
		IIC_Send_Byte((MS5837_ADDR << 1) | 1);
		delay_us(1);
		IIC_Wait_Ack();
		data[0] = IIC_Read_Byte(1);
		delay_us(1);
		data[1] = IIC_Read_Byte(0);
		IIC_Stop();
		ms5837_origin_data.SensorOriginData[i] = (((uint16_t)data[0] << 8) | data[1]);
	}

	crcRead = ms5837_origin_data.SensorOriginData[0] >> 12;
	crcCalculated = crc4(ms5837_origin_data.SensorOriginData);

	if (crcCalculated == crcRead)
	{
		return 1; /* success */
	}
	else
	{
		return 0; /* fail */
	}
}

static void MS5837_Calculate(void)
{
	int32_t dT = 0;
	int64_t SENS = 0;
	int64_t OFF = 0;
	int32_t SENSi = 0;
	int32_t OFFi = 0;
	int32_t Ti = 0;
	int64_t OFF2 = 0;
	int64_t SENS2 = 0;

	dT = ms5837_origin_data.D2 - (uint32_t)ms5837_origin_data.SensorOriginData[5] * 256l;
	if (ms5832_attribute.model == MS5837_02BA)
	{
		SENS = (int64_t)ms5837_origin_data.SensorOriginData[1] * 65536l + ((int64_t)ms5837_origin_data.SensorOriginData[3] * dT) / 128l;
		OFF = (int64_t)ms5837_origin_data.SensorOriginData[2] * 131072l + ((int64_t)ms5837_origin_data.SensorOriginData[4] * dT) / 64l;
		ms5837_origin_data.OriginPressure = (ms5837_origin_data.D1 * SENS / (2097152l) - OFF) / (32768l);
	}
	else
	{
		SENS = (int64_t)ms5837_origin_data.SensorOriginData[1] * 32768l + ((int64_t)ms5837_origin_data.SensorOriginData[3] * dT) / 256l;
		OFF = (int64_t)ms5837_origin_data.SensorOriginData[2] * 65536l + ((int64_t)ms5837_origin_data.SensorOriginData[4] * dT) / 128l;
		ms5837_origin_data.OriginPressure = (int32_t)(ms5837_origin_data.D1 * SENS / (2097152l) - OFF) / (8192l);
	}

	ms5837_origin_data.OriginTemperature = 2000LL + (int64_t)dT * ms5837_origin_data.SensorOriginData[6] / 8388608LL;

	if (ms5832_attribute.model == MS5837_02BA)
	{
		if ((ms5837_origin_data.OriginTemperature / 100) < 20)
		{
			Ti = (int32_t)(11 * (int64_t)dT * (int64_t)dT) / (34359738368LL);
			OFFi = (31 * (ms5837_origin_data.OriginTemperature - 2000) * (ms5837_origin_data.OriginTemperature - 2000)) / 8;
			SENSi = (63 * (ms5837_origin_data.OriginTemperature - 2000) * (ms5837_origin_data.OriginTemperature - 2000)) / 32;
		}
	}
	else
	{
		if ((ms5837_origin_data.OriginTemperature / 100) < 20)
		{
			Ti = (int32_t)((3 * (int64_t)(dT) * (int64_t)(dT)) / (8589934592LL));
			OFFi = (3 * (ms5837_origin_data.OriginTemperature - 2000) * (ms5837_origin_data.OriginTemperature - 2000)) / 2;
			SENSi = (5 * (ms5837_origin_data.OriginTemperature - 2000) * (ms5837_origin_data.OriginTemperature - 2000)) / 8;
			if ((ms5837_origin_data.OriginTemperature / 100) < -15)
			{
				OFFi += 7 * (ms5837_origin_data.OriginTemperature + 1500l) * (ms5837_origin_data.OriginTemperature + 1500l);
				SENSi += 4 * (ms5837_origin_data.OriginTemperature + 1500l) * (ms5837_origin_data.OriginTemperature + 1500l);
			}
		}
		else
		{
			Ti = (int32_t)(2 * (dT * dT) / (137438953472LL));
			OFFi = (1 * (ms5837_origin_data.OriginTemperature - 2000) * (ms5837_origin_data.OriginTemperature - 2000)) / 16;
			SENSi = 0;
		}
	}

	OFF2 = OFF - OFFi;
	SENS2 = SENS - SENSi;

	if (ms5832_attribute.model == MS5837_02BA)
	{
		ms5837_origin_data.OriginTemperature = (ms5837_origin_data.OriginTemperature - Ti);
		ms5837_origin_data.OriginPressure = (((ms5837_origin_data.D1 * SENS2) / 2097152l - OFF2) / 32768l) / 100;
	}
	else
	{
		ms5837_origin_data.OriginTemperature = (ms5837_origin_data.OriginTemperature - Ti);
		ms5837_origin_data.OriginPressure = (((ms5837_origin_data.D1 * SENS2) / 2097152l - OFF2) / 8192l);
	}

	ms5837_data.temperture = ms5837_origin_data.OriginTemperature / ms5832_attribute.tempertureUnit;
	ms5837_data.pressure = ms5837_origin_data.OriginPressure / 10.0f * ms5832_attribute.pressureUnit;
}

static float MS5837_ConvertDepth(void)
{
	return ((ms5837_data.pressure * 100.0f - ms5832_attribute.reference_pressure) / (ms5832_attribute.fluidDensity * 9.80665));
}

void MS5837_Init(int32_t default_density, uint8_t model, int32_t pressure_unit, int32_t temperature_unit, int32_t reference_pressure)
{
	MS5837_SetFluidDensity((float)default_density);
	MS5837_SetModel(model);
	MS5837_SetUnit((float)pressure_unit, (float)temperature_unit);
	MS5837_SetRefPressure((float)reference_pressure);

	while (!MS5837_Check())
	{
		printf("MS5837 Init Failed\r\n");
		delay_ms(100);
	}

	printf("MS5837 Init Successful\r\n");
}

// static void MS5837_Convert(void)
// {
// 	MS5837_WriteByte(MS5837_CONVERT_D1_8192);
// 	delay_ms(20);
// 	ms5837_origin_data.D1 = MS5837_Read4Byte(MS5837_ADC_READ);
// 	ms5837_origin_data.D1 >>= 8;

// 	//    delay_ms(20);

// 	MS5837_WriteByte(MS5837_CONVERT_D2_8192);
// 	delay_ms(20);
// 	ms5837_origin_data.D2 = MS5837_Read4Byte(MS5837_ADC_READ);
// 	ms5837_origin_data.D2 >>= 8;
// }

// void MS5837_getData(float* output_temperature, float* output_pressure)
// {
// 	//    MS5837_Convert();
// 	MS5837_WriteByte(MS5837_CONVERT_D1_8192);
// 	delay_ms(20);
// 	ms5837_origin_data.D1 = MS5837_Read4Byte(MS5837_ADC_READ);
// 	ms5837_origin_data.D1 >>= 8;

// 	//    delay_ms(20);

// 	MS5837_WriteByte(MS5837_CONVERT_D2_8192);
// 	delay_ms(20);
// 	ms5837_origin_data.D2 = MS5837_Read4Byte(MS5837_ADC_READ);
// 	ms5837_origin_data.D2 >>= 8;

// 	MS5837_Calculate();

// 	*output_temperature = ms5837_data.temperture;
// 	*output_pressure = ms5837_data.pressure;
// }

void MS5837_GetDataTask_cb(MultiTimer* timer, void* userData)
{
	switch (MS5837_STATE)
	{
	case MS5837_STATE_CONVERT_D1:
		MS5837_WriteByte(MS5837_CONVERT_D1_8192);
		MS5837_STATE = MS5837_STATE_CONVERT_D2;
		break;
	case MS5837_STATE_CONVERT_D2:
		ms5837_origin_data.D1 = MS5837_Read4Byte(MS5837_ADC_READ);
		ms5837_origin_data.D1 >>= 8;
		MS5837_WriteByte(MS5837_CONVERT_D2_8192);
		MS5837_STATE = MS5837_STATE_CALCULATE;
		break;
	case MS5837_STATE_CALCULATE:
		ms5837_origin_data.D2 = MS5837_Read4Byte(MS5837_ADC_READ);
		ms5837_origin_data.D2 >>= 8;
		MS5837_Calculate();
		MS5837_STATE = MS5837_STATE_CONVERT_D1;
		break;
	default:
		MS5837_STATE = MS5837_STATE_CONVERT_D1;
		break;
	}
	MultiTimerRestart(timer, (uint32_t)(userData));
}

// void MS5837_GetData(float* output_temperature, float* output_pressure)
// {
// 	*output_temperature = ms5837_data.temperture;
// 	*output_pressure = ms5837_data.pressure;
// }
// water_data

void MS5837_GetData(MS5837_Data_t* output_data)
{
	output_data->temperture = ms5837_data.temperture;
	output_data->pressure = ms5837_data.pressure;
}

void MS5837_ReadDepth(uint8_t is_filtered, float* output_depth)
{
	float w_depth;
	float temp;

	w_depth = MS5837_ConvertDepth();

	if (is_filtered)
	{
		temp = ms5837_fir.filter[ms5837_fir.cnt];
		ms5837_fir.filter[ms5837_fir.cnt] = w_depth;
		ms5837_fir.sum += ms5837_fir.filter[ms5837_fir.cnt] - temp;
		*output_depth = ms5837_fir.sum / 10.0f;
		ms5837_fir.cnt++;
	}
	else //not filtered
	{
		*output_depth = w_depth;
	}

	if (ms5837_fir.cnt == 10)
	{
		ms5837_fir.cnt = 0;
	}
}


