#include "sonar.h"


uint8_t P30Send[P30_UART_TXLen] = { 0 };








 
//    
//static void SonarSendString(USART_TypeDef* uart, uint8_t* str)
//{
//	while(*str)
//	{
//		while(USART_GetFlagStatus(uart, USART_FLAG_TXE) == RESET);
//		USART_SendData(uart, *str);
//        str++;
//	}
//}

//void InitP30(void)
//{
//	//42 52 02 00 78 05 00 00 BB 04 D2 01
//	P30Send[0] = 0x42;
//	P30Send[1] = 0x52;
//	P30Send[2] = 0x02;
//	P30Send[3] = 0x00;
//	P30Send[4] = 0x05;
//	P30Send[5] = 0x00;
//	P30Send[6] = 0x00;
//	P30Send[7] = 0x00;
//	P30Send[8] = 0xBB;
//	P30Send[9] = 0x04;
//	P30Send[10] = 0xD2;
//	P30Send[11] = 0x01;

//    SonarSendString(USART2, P30Send);
//}


//SonarData_t ReceiveP30(void)
//{
//	SonarData_t RevP30;

//	uint8_t FrameState = 0;
//	uint8_t Bytenum = 0;
//	uint8_t CheckSum = 0;
//	uint8_t datahex[11];

//	for (uint8_t i = 0; i < 33; i++)
//	{
//		if (FrameState == 0)
//		{
//			if ((P30Receive[i] == 0x42) && (Bytenum == 0))
//			{
//				CheckSum = P30Receive[i];
//				Bytenum = 1;
//				continue;
//			}
//			else if ((P30Receive[i] == 0x52) && (Bytenum == 1))
//			{
//				CheckSum += P30Receive[i];
//				Bytenum = 2;
//				FrameState = 1;
//				continue;
//			}
//		}
//		else if (FrameState == 1)
//		{
//			if (Bytenum < 13)
//			{
//				datahex[Bytenum - 2] = P30Receive[i];
//				CheckSum += P30Receive[i];
//				Bytenum++;
//			}
//			else
//			{
//				if (P30Receive[i] == (CheckSum & 0xFF))
//				{
//					RevP30.Confidence = (datahex[10]) | (datahex[11]);
//					RevP30.DepthToBottom = (((datahex[6] << 24)
//							| (datahex[7] << 16) | (datahex[8] << 8)
//							| (datahex[9])) / 1000);
//				}
//				CheckSum = 0;
//				Bytenum = 0;
//				FrameState = 0;
//			}
//		}
//	}

//	__HAL_UART_ENABLE_IT(&GP30_UART, UART_IT_IDLE);
//	HAL_UART_Receive_DMA(&GP30_UART, P30Receive, P30_UART_RXLen);

//	return RevP30;
//}

