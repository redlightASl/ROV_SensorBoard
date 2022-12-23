#ifndef __SONAR_P30_H
#define __SONAR_P30_H
#include <stdint.h>
#include <sys.h>

#define BOTTOM_SONAR 0


#define P30_UART_TXLen 12
#define P30_UART_RXLen 33

struct Sonar_Attribute
{
	uint8_t Direction; //Sonar's Front Direction
};
typedef struct Sonar_Attribute Sonar_Attribute_t;

struct SonarData
{
    uint32_t SonarHeight;
    uint32_t SonarDistance;
    uint8_t Confidence;
};
typedef struct SonarData SonarData_t;





#endif


