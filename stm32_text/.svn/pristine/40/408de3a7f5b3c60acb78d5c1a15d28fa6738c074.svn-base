#include "eeprom.h" 
#include "delay.h"
#include "usart.h"
#include "myiic.h"
#include "debug_usart.h"
/*
 * 在CPLD指定地址读出一个数据
 * ReadAddr:开始读数的地址  
 * 返回值  :读到的数据
 */

#define AT24C02_LEN  (256)

u8 E2PROM_ReadOneByte(u8 ReadAddr)
{
	u8 temp = 0;

    IIC_Start(IIC_E_BUS0);
    IIC_SendByte(IIC_E_BUS0, 0xA0); //发送器件地址0XA0,写数据
	IIC_WaitAck(IIC_E_BUS0);
    IIC_SendByte(IIC_E_BUS0, ReadAddr); //发送低地址
	IIC_WaitAck(IIC_E_BUS0);
	IIC_Start(IIC_E_BUS0);
    IIC_SendByte(IIC_E_BUS0, 0xA1); //进入接收模式
	IIC_WaitAck(IIC_E_BUS0);
    temp = IIC_ReadByte(IIC_E_BUS0, 0);
    IIC_Stop(IIC_E_BUS0);
	return temp;
}

/*
 * 指定地址写入一个数据
 * WriteAddr  :写入数据的目的地址    
 * DataToWrite:要写入的数据
 */
void E2PROM_WriteOneByte(u8 WriteAddr, u8 DataToWrite)
{
    IIC_Start(IIC_E_BUS0);  
    IIC_SendByte(IIC_E_BUS0, 0xA0); //发送器件地址0XA0,写数据 
	IIC_WaitAck(IIC_E_BUS0);	   
    IIC_SendByte(IIC_E_BUS0, WriteAddr); //发送地址
	IIC_WaitAck(IIC_E_BUS0); 	 										  		   
	IIC_SendByte(IIC_E_BUS0, DataToWrite); 							   
	IIC_WaitAck(IIC_E_BUS0);  		    	   
    IIC_Stop(IIC_E_BUS0); 
	delay_ms(10);	 
}

/*
 * 指定地址开始写入长度为Len的数据
 * WriteAddr  :开始写入的地址  
 * DataToWrite:数据数组首地址
 * Len        :要写入数据的长度
 */
void E2PROM_WriteLenByte(u8 WriteAddr, u8 *pBuf, u8 len)
{
	u8 t;

    if((AT24C02_LEN < len))
    {
        DebugPrint("the input param is err\n");
        return;
    }

	for(t = 0; t < len; t++)
	{
		E2PROM_WriteOneByte(WriteAddr + t, (*(pBuf+t)) & 0xff);
	}
}

/*
 * 指定地址开始读出长度为Len的数据
 * ReadAddr   :开始读出的地址 
 * 返回值     :数据
 * Len        :要读出数据的长度
 */
u8 E2PROM_ReadLenByte(u8 ReadAddr, u8 *pBuf, u8 len)
{
	u8 t    = 0;

	for(t = 0; t < len; t++)
	{
		*(pBuf+t) = E2PROM_ReadOneByte(ReadAddr + t);
	}

	return 0;
}

u8 bsp_e2promRead(u8 *pSrcData, u8 *pResData, u16 *resDataLen)
{
    u8  addr = 0;
    u8  len = 0;

    addr = pSrcData[0];
    len = pSrcData[1];
    E2PROM_ReadLenByte(addr, pResData, len);
    *resDataLen = len;

    return 0;
}

int bsp_e2promWrte(u8 *pSrcData)
{
    u8 len = 0;
    u8  addr = 0;
    u8 *pBuf;

    addr = pSrcData[0];
    len = pSrcData[1];
    pBuf = &pSrcData[2];

    E2PROM_WriteLenByte(addr, pBuf, len);

    return 0;
}


