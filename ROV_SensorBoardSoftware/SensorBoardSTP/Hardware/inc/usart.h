#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

//ʹ�ܣ�1��/��ֹ��0������1����
#define EN_USART1_RX 1

//�������������ֽ���
#define USART_REC_LEN 200

//�����������������ֽ���
#define SENSOR_MAX_RECV_LEN 128

extern u8 SENSOR_RX_BUFFER[SENSOR_MAX_RECV_LEN]; //Rx Buffer
extern u16 SENSOR_RX_FLAG; //Rx Flag

extern u8 USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA; //����״̬���	

void uart_init(u32 bound);
void uart_sensor_init(u32 bound);

#endif


