#include "sht20_iic.h"
#include "delay.h"


//初始化IIC
void SHT20_IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //使能GPIOB时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11); //PB10,PB11 输出高
}

u8 SHT20_SoftReset(void)
{
	u8 ret = 0;
	SHT20_IIC_Start();
	SHT20_IIC_Send_Byte((SHT2x_I2C_ADDR << 1) | 0);
	ret = SHT20_IIC_Wait_Ack();
	if (ret)
	{
		return 0; //error
	}
	SHT20_IIC_Send_Byte(SHT2x_SOFT_RESET);
	ret = SHT20_IIC_Wait_Ack();
	if (ret)
	{
		return 0; //error
	}
	SHT20_IIC_Stop();
	return 1;
}

u8 SHT20_ReadUserReg(void)
{
	u8 res = 0;

	SHT20_IIC_Start();
	SHT20_IIC_Send_Byte((SHT2x_I2C_ADDR << 1) | 0);
	SHT20_IIC_Wait_Ack();
	SHT20_IIC_Send_Byte(SHT2x_READ_REG);
	SHT20_IIC_Wait_Ack();
	SHT20_IIC_Start();
	SHT20_IIC_Send_Byte((SHT2x_I2C_ADDR << 1) | 1);
	SHT20_IIC_Wait_Ack();
	res = SHT20_IIC_Read_Byte(0);
	SHT20_IIC_Stop();

	return res;
}

u8 SHT20_SetResolution(SHT20_Resolution_t res)
{
	u8 ret = 0;
	u8 val = 0;
	val = SHT20_ReadUserReg();
	val = (val & 0x7e) | res;
	//    u8 temp[2] = { SHT2x_WRITE_REG, val };

	SHT20_IIC_Start();
	SHT20_IIC_Send_Byte((SHT2x_I2C_ADDR << 1) | 0);
	ret = SHT20_IIC_Wait_Ack();
	if (ret)
	{
		return 0;
	}
	SHT20_IIC_Send_Byte(SHT2x_WRITE_REG);
	ret = SHT20_IIC_Wait_Ack();
	if (ret)
	{
		return 0;
	}
	SHT20_IIC_Send_Byte(val);
	ret = SHT20_IIC_Wait_Ack();
	if (ret)
	{
		return 0;
	}
	SHT20_IIC_Stop();
	return 1;
}

//产生IIC起始信号
void SHT20_IIC_Start(void)
{
	SHT20_SDA_OUT(); //sda线输出
	SHT20_IIC_SDA = 1;
	SHT20_IIC_SCL = 1;
	delay_us(4);
	SHT20_IIC_SDA = 0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	SHT20_IIC_SCL = 0;//钳住I2C总线，准备发送或接收数据 
}

//产生IIC停止信号
void SHT20_IIC_Stop(void)
{
	SHT20_SDA_OUT();//sda线输出
	SHT20_IIC_SCL = 0;
	SHT20_IIC_SDA = 0;//STOP:when CLK is high DATA change form low to high
	delay_us(4);
	SHT20_IIC_SCL = 1;
	SHT20_IIC_SDA = 1;//发送I2C总线结束信号
	delay_us(4);
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 SHT20_IIC_Wait_Ack(void)
{
	u16 ucErrTime = 0;
	SHT20_SDA_IN();      //SDA设置为输入  
	SHT20_IIC_SDA = 1;delay_us(1);
	SHT20_IIC_SCL = 1;delay_us(1);
	while (SHT20_READ_SDA)
	{
		ucErrTime++;
		if (ucErrTime > SHT2x_TIMEOUT)
		{
			SHT20_IIC_Stop();
			return 1;
		}
	}
	SHT20_IIC_SCL = 0;//时钟输出0 	   
	return 0;
}

//产生ACK应答
void SHT20_IIC_Ack(void)
{
	SHT20_IIC_SCL = 0;
	SHT20_SDA_OUT();
	SHT20_IIC_SDA = 0;
	delay_us(2);
	SHT20_IIC_SCL = 1;
	delay_us(2);
	SHT20_IIC_SCL = 0;
}

//不产生ACK应答		    
void SHT20_IIC_NAck(void)
{
	SHT20_IIC_SCL = 0;
	SHT20_SDA_OUT();
	SHT20_IIC_SDA = 1;
	delay_us(2);
	SHT20_IIC_SCL = 1;
	delay_us(2);
	SHT20_IIC_SCL = 0;
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void SHT20_IIC_Send_Byte(u8 txd)
{
	u8 t;
	SHT20_SDA_OUT();
	SHT20_IIC_SCL = 0;//拉低时钟开始数据传输
	for (t = 0;t < 8;t++)
	{
		SHT20_IIC_SDA = (txd & 0x80) >> 7;
		txd <<= 1;
		delay_us(2);   //对TEA5767这三个延时都是必须的
		SHT20_IIC_SCL = 1;
		delay_us(2);
		SHT20_IIC_SCL = 0;
		delay_us(2);
	}
}

//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 SHT20_IIC_Read_Byte(unsigned char ack)
{
	unsigned char i, receive = 0;
	SHT20_SDA_IN();//SDA设置为输入
	for (i = 0;i < 8;i++)
	{
		SHT20_IIC_SCL = 0;
		delay_us(2);
		SHT20_IIC_SCL = 1;
		receive <<= 1;
		if (SHT20_READ_SDA)receive++;
		delay_us(1);
	}
	if (!ack)
		SHT20_IIC_NAck();//发送nACK
	else
		SHT20_IIC_Ack(); //发送ACK   
	return receive;
}


