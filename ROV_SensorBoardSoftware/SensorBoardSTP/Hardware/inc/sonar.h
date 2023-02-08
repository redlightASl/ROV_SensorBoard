#ifndef __SONAR_P30_H
#define __SONAR_P30_H
#include <stdint.h>
#include "sys.h"
#include "MultiTimer.h"

//states
#define P30_STATE_IDLE 1
#define P30_STATE_START 2
#define P30_STATE_RECV 3
#define P30_STATE_SUMCHECK 4

#define P30_UART_TXLen 12
#define P30_UART_RXLen 33


//传感器串口最大接收字节数
#define SENSOR_MAX_RECV_LEN 128

struct SonarData
{
    uint32_t SonarHeight;
    uint8_t Confidence;
};
typedef struct SonarData SonarData_t;

extern MultiTimer P30_recv_timer;
extern uint8_t SONAR_RX_FLAG;

void P30_GetDataTask_cb(MultiTimer* timer, void* userData);
void P30_Init(void);
void P30_Request(void);
void P30_ReadData(SonarData_t* data);
void P30_Recv_ISR(uint8_t* res, uint32_t CheckSum);

#endif


