#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

//使能（1）/禁止（0）串口1接收
#define EN_USART1_RX 1

//主串口最大接收字节数
#define USART_REC_LEN 200

extern u8 USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA; //接收状态标记	

void uart_init(u32 bound);
void uart_sensor_init(u32 bound);

#endif


