#ifndef __SHT20_H
#define __SHT20_H
#include <stdint.h>
#include "sys.h"
#include "MultiTimer.h"

//states
#define SHT20_STATE_SEND_CMD1 1
#define SHT20_STATE_GET_TEMP 2
#define SHT20_STATE_SEND_CMD2 3
#define SHT20_STATE_GET_HUMI 4
#define SHT20_STATE_CALCULATE 5

struct SHT20_OriginData
{
	uint16_t RawTemperature;
	uint16_t RawRelativeHumidity;
};
typedef struct SHT20_OriginData SHT20_OriginData_t;

struct SHT20_Data
{
	float temperture;
	float humidity;
};
typedef struct SHT20_Data SHT20_Data_t;

// extern volatile MultiTimer SHT20_recv_timer;
extern MultiTimer SHT20_recv_timer;

void SHT20_GetDataTask_cb(MultiTimer* timer, void* userData);

void SHT20_Init(void);
void SHT20_GetData(SHT20_Data_t* sht20_data);
float SHT20_CelsiusToKelvin(float celsius);


#endif
