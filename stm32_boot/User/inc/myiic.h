#ifndef __MYIIC_H
#define __MYIIC_H
#include "sys.h"

/* IO方向设置 */
#define SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF; GPIOB->CRL|=(u32)8<<28;}
#define SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF; GPIOB->CRL|=(u32)3<<28;}

/* IO操作函数 */
#define IIC_SCL    PBout(6) //SCL
#define IIC_SDA    PBout(7) //SDA	 
#define READ_SDA   PBin(7)  //输入SDA 

/* IIC所有操作函数 */
extern void i2c_init(void);
extern void IIC_Start(void);
extern void IIC_Stop(void);
extern void IIC_SendByte(u8 txd);
extern void IIC_Ack(void);
extern void IIC_NAck(void);
extern u8   IIC_ReadByte(unsigned char ack);
extern u8   IIC_WaitAck(void);

extern void IIC_Write_One_Byte(u8 daddr, u8 addr, u8 data);
extern u8 IIC_Read_One_Byte(u8 daddr, u8 addr);	  

/* I2C寄存器个数 */
#define I2C_REG_NUM			0xbd

#endif

