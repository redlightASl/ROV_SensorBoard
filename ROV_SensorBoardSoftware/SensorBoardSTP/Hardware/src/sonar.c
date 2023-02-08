#include <stdio.h>
#include "delay.h"
#include "sonar.h"
#include "MultiTimer.h"

SonarData_t RevP30;
static uint8_t P30_SendBuffer[P30_UART_TXLen] = { 0x42, 0x52, 0x02, 0x00, 0x78, 0x05, 0x00, 0x00, 0x14, 0x05, 0x2C, 0x01 };
uint8_t SONAR_RX_FLAG = 0; //Rx Flag
MultiTimer P30_recv_timer;

static inline void SonarSendByte(USART_TypeDef* uart, uint8_t data)
{
    USART_SendData(uart, data);
    while (USART_GetFlagStatus(uart, USART_FLAG_TC) == RESET);
}

static void SonarSendString(USART_TypeDef* uart, uint8_t* str, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++)
    {
        SonarSendByte(uart, str[i]);
    }
    while (USART_GetFlagStatus(uart, USART_FLAG_TC) == RESET);
}

void P30_Init(void)
{
    //consequence: 42 52 02 00 78 05 00 00 14 05 2C 01
    //stop consequence: 42 52 02 00 79 05 00 00 14 05 2D 01
    //set voice speed=1400: 42 52 04 00 EA 03 00 00 C0 5C 15 00 B6 02
    //42 52 02 00 78 05 00 00 BB 04 D2 01

    SonarSendByte(USART2, 0x00);

    // P30_SendBuffer[0] = 0x42;
    // P30_SendBuffer[1] = 0x52;
    // P30_SendBuffer[2] = 0x02;
    // P30_SendBuffer[3] = 0x00;
    // P30_SendBuffer[4] = 0x78;
    // P30_SendBuffer[5] = 0x05;
    // P30_SendBuffer[6] = 0x00;
    // P30_SendBuffer[7] = 0x00;
    // P30_SendBuffer[8] = 0x14;
    // P30_SendBuffer[9] = 0x05;
    // P30_SendBuffer[10] = 0x2C;
    // P30_SendBuffer[11] = 0x01;

    // for (uint8_t i = 0;i < 12;i++)
    // {
    //     // SonarSendByte(USART2, P30_SendBuffer[i]);
    //     SonarSendByte(USART2, P30_SendBuffer[i]);
    // }

    // SonarSendString(USART2, P30_SendBuffer, P30_UART_TXLen);
    // SonarSendString(USART2, P30_SendBuffer, 10);
    P30_Request();
    delay_ms(10);
    printf("SonarP30 Init Successful\r\n");
}

void P30_Request(void)
{
    //request: 42 52 00 00 BB 04 00 00 53 01
    uint8_t P30_RequestBuffer[10] = { 0 };
    P30_RequestBuffer[0] = 0x42;
    P30_RequestBuffer[1] = 0x52;
    P30_RequestBuffer[2] = 0x00;
    P30_RequestBuffer[3] = 0x00;
    P30_RequestBuffer[4] = 0xBB;
    P30_RequestBuffer[5] = 0x04;
    P30_RequestBuffer[6] = 0x00;
    P30_RequestBuffer[7] = 0x00;
    P30_RequestBuffer[8] = 0x53;
    P30_RequestBuffer[9] = 0x01;

    for (uint8_t i = 0;i < 10;i++)
    {
        SonarSendByte(USART2, P30_RequestBuffer[i]);
    }
}

void P30_ReadData(SonarData_t* data)
{
    if ((RevP30.Confidence > 70) && (RevP30.Confidence <= 100))
    {
        data->Confidence = RevP30.Confidence;
        data->SonarHeight = RevP30.SonarHeight;
    }
    else
    {
        data->Confidence = 0;
        data->SonarHeight = RevP30.SonarHeight;
    }

}

void P30_GetDataTask_cb(MultiTimer* timer, void* userData)
{
    P30_Request();

    // if (SONAR_RX_FLAG)
    // {
    //     SONAR_RX_FLAG = 0;
    //     switch (P30_STATE)
    //     {
    //     case P30_STATE_IDLE:
    //         if (Bytenum == 1)
    //         {
    //             P30_STATE = P30_STATE_START1;
    //         }
    //         break;
    //     case P30_STATE_START1:
    //         if (P30_RECV_DATA == 0x42)
    //         {
    //             CheckSum = P30_RECV_DATA;
    //             P30_STATE = P30_STATE_START2;
    //         }
    //         break;
    //     case P30_STATE_START2:
    //         if (P30_RECV_DATA == 0x52)
    //         {
    //             CheckSum += P30_RECV_DATA;
    //             P30_STATE = P30_STATE_RECV;
    //         }
    //         break;
    //     case P30_STATE_RECV:
    //         if (Bytenum < 13)
    //         {
    //             datahex[Bytenum - 2] = P30_RECV_DATA;
    //             CheckSum += P30_RECV_DATA;
    //         }
    //         else
    //         {
    //             if (P30_RECV_DATA == (CheckSum & 0xFF))
    //             {
    //                 P30_STATE = P30_STATE_SUMCHECK;
    //             }
    //             else
    //             {
    //                 P30_STATE = P30_STATE_IDLE;
    //                 CheckSum = 0;
    //                 Bytenum = 0;
    //             }
    //         }
    //         break;
    //     case P30_STATE_SUMCHECK:
    //         RevP30.Confidence = (datahex[10]) | (datahex[11]);
    //         RevP30.SonarHeight = (((datahex[6] << 24)
    //             | (datahex[7] << 16) | (datahex[8] << 8)
    //             | (datahex[9])) / 1000);
    //         CheckSum = 0;
    //         Bytenum = 0;
    //         P30_STATE = P30_STATE_IDLE;
    //         break;
    //     default:
    //         break;
    //     }
    // }

    MultiTimerRestart(timer, (uint32_t)(userData));
}




// static uint8_t FrameState = 0;
// static uint8_t Bytenum = 0;
// static uint32_t CheckSum = 0;
// static uint8_t datahex[12];

void P30_Recv_ISR(uint8_t* res, uint32_t CheckSum)
{
    // Bytenum++;
    // P30_RECV_DATA = res;


    // if ((((uint16_t)res[11] << 8) | ((uint16_t)res[12])) == ((uint16_t)(CheckSum & 0x0000FFFF)))
    // {
    RevP30.Confidence = res[10];
    RevP30.SonarHeight = ((uint32_t)res[9] << 24) |
        ((uint32_t)res[8] << 16) |
        ((uint32_t)res[7] << 8) |
        ((uint32_t)res[6]);
    SONAR_RX_FLAG = 1;
    // printf("decoded data: %d, %d\r\n", RevP30.Confidence, RevP30.SonarHeight);
// }

// if (FrameState == 0)
// {
//     if ((res == 0x42) && (Bytenum == 0))
//     {
//         CheckSum = res;
//         Bytenum = 1;
//     }
//     else if ((res == 0x52) && (Bytenum == 1))
//     {
//         CheckSum += res;
//         Bytenum = 2;
//         FrameState = 1;
//     }
// }
// else if (FrameState == 1)
// {
//     if (Bytenum < 13)
//     {
//         datahex[Bytenum - 2] = res;
//         CheckSum += res;
//         Bytenum++;
//     }
//     else
//     {
//         if (res == (CheckSum & 0x000000FF))
//         {
//             RevP30.Confidence = (datahex[10]) | (datahex[11]);
//             RevP30.SonarHeight = (((((uint32_t)(datahex[6])) << 24) |
//                 (((uint32_t)(datahex[7])) << 16) |
//                 (((uint32_t)(datahex[8])) << 8) |
//                 ((uint32_t)(datahex[9]))) / 1000);

//             printf("decoded data: %d, %d\r\n", RevP30.Confidence, RevP30.SonarHeight);
//             SONAR_RX_FLAG = 1;
//         }
//         CheckSum = 0;
//         Bytenum = 0;
//         FrameState = 0;
//     }
// }
}

