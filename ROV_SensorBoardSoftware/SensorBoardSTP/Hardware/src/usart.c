#include "sys.h"
#include "usart.h"	  
#include "sonar.h"


//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE
{
	int handle;

};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x)
{
	x = x;
}
//重定义fputc函数 
int fputc(int ch, FILE* f)
{
	while ((USART1->SR & 0X40) == 0); //循环发送,直到发送完毕   
	USART1->DR = (u8)ch;
	return ch;
}
#endif 


//读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节

//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA = 0; //接收状态标记	  


void uart_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1, ENABLE);
}


void uart_sensor_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	//USART2_TX PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//USART2_RX	PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	// GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//NVIC
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//USART2
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	// USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
	USART_Cmd(USART2, ENABLE);
}

void USART1_IRQHandler(void)
{
	u8 Res;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res = USART_ReceiveData(USART1); //读取接收到的数据
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);

		if ((USART_RX_STA & 0x8000) == 0) //接收未完成
		{
			if (USART_RX_STA & 0x4000) //接收到了0x0d
			{
				if (Res != 0x0a)USART_RX_STA = 0; //接收错误,重新开始
				else USART_RX_STA |= 0x8000; //接收完成
			}
			else //还没收到0X0D
			{
				if (Res == 0x0d)
				{
					USART_RX_STA |= 0x4000;
				}
				else
				{
					USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res;
					USART_RX_STA++;
					if (USART_RX_STA > (USART_REC_LEN - 1))
					{
						USART_RX_STA = 0; //接收数据错误,重新开始接收
					}
				}
			}
		}
	}
}


static uint8_t Bytenum = 0;
static uint32_t CheckSum = 0;
static uint8_t USART2_RX_BUF[13] = {0};

static uint8_t P30_STATE = P30_STATE_IDLE;

void USART2_IRQHandler(void)
{
	uint8_t Res;
	// if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	// {
	// 	Res = USART_ReceiveData(USART2); //读取接收到的数据
	// 	printf("%d", Res);
	// 	// P30_Recv_ISR(Res);
	// 	USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	// }

	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		Res = USART_ReceiveData(USART2); //读取接收到的数据
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);

		switch (P30_STATE)
		{
		case P30_STATE_IDLE:
			if (Res == 0x42)
			{
				CheckSum = Res;
				P30_STATE = P30_STATE_START;
			}
			else
			{
				P30_STATE = P30_STATE_IDLE;
			}
			break;
		case P30_STATE_START:
			if (Res == 0x52)
			{
				CheckSum += Res;
				Bytenum = 0;
				P30_STATE = P30_STATE_RECV;
			}
			else
			{
				P30_STATE = P30_STATE_IDLE;
			}
			break;
		case P30_STATE_RECV:
			if (Bytenum < 13)
			{
				CheckSum += Res;
				USART2_RX_BUF[Bytenum++] = Res;
				P30_STATE = P30_STATE_RECV;
			}
			else
			{
				P30_Recv_ISR(USART2_RX_BUF, 0x00);
				P30_STATE = P30_STATE_IDLE;
			}
		default:
			break;
		}

		// printf("current_state: %d, data: %d\r\n", P30_STATE, Res);
		// USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
	// else if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	// {
	// 	P30_Recv_ISR(USART2_RX_BUF, 0x00);
	// 	USART_ClearITPendingBit(USART2, USART_IT_IDLE);
	// }
}

