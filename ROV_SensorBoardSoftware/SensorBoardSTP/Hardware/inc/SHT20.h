#ifndef __SHT20_H
#define __SHT20_H
#include "sys.h"

void SHT20_Init(void);
void SHT20_GetData(float* temp, float* rh);
float SHT20_CelsiusToKelvin(float celsius);


#endif
