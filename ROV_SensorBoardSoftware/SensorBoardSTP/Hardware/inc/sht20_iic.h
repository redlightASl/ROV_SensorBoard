#ifndef __SHT20_IIC_H
#define __SHT20_IIC_H
#include "sys.h"

#define SHT2x_I2C_ADDR			0x40
#define SHT2x_HOLD_MASTER		1
#define SHT2x_READ_TEMP_HOLD	0xe3
#define	SHT2x_READ_RH_HOLD		0xe5
#define SHT2x_READ_TEMP_NOHOLD	0xf3
#define SHT2x_READ_RH_NOHOLD	0xf5
#define	SHT2x_WRITE_REG			0xe6
#define SHT2x_READ_REG			0xe7
#define SHT2x_SOFT_RESET		0xfe
#define SHT2x_TIMEOUT			250

//PB10=SCL PB11=SDA
//IO方向设置
#define SHT20_SDA_IN()  {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=(u32)8<<12;}
#define SHT20_SDA_OUT() {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=(u32)3<<12;}

//IO操作函数
#define SHT20_IIC_SCL    PBout(10) //SCL
#define SHT20_IIC_SDA    PBout(11) //SDA
#define SHT20_READ_SDA   PBin(11) //输入SDA


typedef enum SHT20_Resolution {
	RES_14_12 = 0x00,
	RES_12_8 = 0x01,
	RES_13_10 = 0x80,
	RES_11_11 = 0x81,
} SHT20_Resolution_t;


void SHT20_IIC_Init(void);
u8 SHT20_SoftReset(void);
u8 SHT20_ReadUserReg(void);
u8 SHT20_SetResolution(SHT20_Resolution_t res);
u16 SHT20_GetRaw(u8 cmd);

void SHT20_IIC_Start(void);
void SHT20_IIC_Stop(void);
u8 SHT20_IIC_Wait_Ack(void);
void SHT20_IIC_Ack(void);
void SHT20_IIC_NAck(void);
void SHT20_IIC_Send_Byte(u8 txd);
u8 SHT20_IIC_Read_Byte(unsigned char ack);

#endif
