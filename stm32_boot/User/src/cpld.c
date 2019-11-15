#include "cpld.h" 
#include "delay.h"
#include "usart.h"
#include "myiic.h"
/*
 * 在CPLD指定地址读出一个数据
 * ReadAddr:开始读数的地址  
 * 返回值  :读到的数据
 */
u8 CPLD_ReadOneByte(u16 ReadAddr)
{				  
	u8 temp=0;		  	    																 

    IIC_Start();  
    IIC_SendByte(0xB8); //发送器件地址0XA0,写数据 	 
	IIC_WaitAck(); 
    IIC_SendByte(ReadAddr); //发送低地址
	IIC_WaitAck();	    
	IIC_Start();  	 	   
	IIC_SendByte(0XB9); //进入接收模式			   
	IIC_WaitAck();	 
    temp=IIC_ReadByte(0);		   
    IIC_Stop();                 	    
	return temp;
}

/*
 * 指定地址写入一个数据
 * WriteAddr  :写入数据的目的地址    
 * DataToWrite:要写入的数据
 */
void CPLD_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{				   	  	    																 
    IIC_Start();  
    IIC_SendByte(0xB8); //发送器件地址0XA0,写数据 
	IIC_WaitAck();	   
    IIC_SendByte(WriteAddr); //发送低地址
	IIC_WaitAck(); 	 										  		   
	IIC_SendByte(DataToWrite); 							   
	IIC_WaitAck();  		    	   
    IIC_Stop(); 
	delay_ms(10);	 
}

/*
 * 指定地址开始写入长度为Len的数据
 * 该函数用于写入16bit或者32bit的数据.
 * WriteAddr  :开始写入的地址  
 * DataToWrite:数据数组首地址
 * Len        :要写入数据的长度2,4
 */
void CPLD_WriteLenByte(u16 WriteAddr, u32 DataToWrite, u8 Len)
{  	
	u8 t;
	for(t = 0; t < Len; t++)
	{
		CPLD_WriteOneByte(WriteAddr + t, (DataToWrite >> (8 * t)) & 0xff);
	}												    
}

#if 0

/*
 * 指定地址开始读出长度为Len的数据
 * 该函数用于读出16bit或者32bit的数据.
 * ReadAddr   :开始读出的地址 
 * 返回值     :数据
 * Len        :要读出数据的长度2,4
 */
u32 CPLD_ReadLenByte(u16 ReadAddr, u8 Len)
{  	
	u8 t;
	u32 temp = 0;
	for(t = 0; t < Len; t++)
	{
		temp <<= 8;
		temp += AT24CXX_ReadOneByte(ReadAddr + Len - t - 1); 	 				   
	}

	return temp;												    
}


/*
 * 检查CPLD是否正常
 * 这里用了CPLD的最后一个地址(255)来存储标志字.
 * 返回1:检测失败
 * 返回0:检测成功
 */
u8 CPLD_Check(void)
{
	u8 temp;
	temp = AT24CXX_ReadOneByte(255);//避免每次开机都写AT24CXX
	if(temp == 0X55)
    {
        return 0;		   
    }
	else//排除第一次初始化的情况
	{
		AT24CXX_WriteOneByte(255,0X55);
	    temp = AT24CXX_ReadOneByte(255);	  
		if(temp == 0x55)
        {
            return 0;
        }
	}

	return 1;											  
}
#endif

/*
 * 在CPLD里面的指定地址开始读出指定个数的数据
 * ReadAddr :开始读出的地址 对24c02为0~255
 * pBuffer  :数据数组首地址
 * NumToRead:要读出数据的个数
 */
void CPLD_Read(u16 ReadAddr, u8 *pBuffer, u16 NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++ = CPLD_ReadOneByte(ReadAddr++);	
		NumToRead--;
	}
}

/*
 * 在CPLD里面的指定地址开始写入指定个数的数据
 * WriteAddr :开始写入的地址 对24c02为0~255
 * pBuffer   :数据数组首地址
 * NumToWrite:要写入数据的个数
 */
void CPLD_Write(u16 WriteAddr, u8 *pBuffer, u16 NumToWrite)
{
	while(NumToWrite--)
	{
		CPLD_WriteOneByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}

s8 CPLD_ReadProc(u8 *pData, u16 len, u8 *pResData, u16 *resDataLen)
{
	s8 ret = 0;
	
	USART_WriteData(UART_CONTROL_3, pData, len);
	ret = USART_ReadData(UART_CONTROL_3, pResData, 1);

	*resDataLen = 1;
	
	return ret;
}

s8 CPLD_WriteProc(u8 *pData, u16 len)
{
    USART_SendString(USART3, pData, len);

	return 0;
}
