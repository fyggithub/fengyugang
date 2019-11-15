#include "cpld.h" 
#include "delay.h"
#include "usart.h"
#include "myiic.h"
/*
 * ��CPLDָ����ַ����һ������
 * ReadAddr:��ʼ�����ĵ�ַ  
 * ����ֵ  :����������
 */
u8 CPLD_ReadOneByte(u16 ReadAddr)
{				  
	u8 temp=0;		  	    																 

    IIC_Start();  
    IIC_SendByte(0xB8); //����������ַ0XA0,д���� 	 
	IIC_WaitAck(); 
    IIC_SendByte(ReadAddr); //���͵͵�ַ
	IIC_WaitAck();	    
	IIC_Start();  	 	   
	IIC_SendByte(0XB9); //�������ģʽ			   
	IIC_WaitAck();	 
    temp=IIC_ReadByte(0);		   
    IIC_Stop();                 	    
	return temp;
}

/*
 * ָ����ַд��һ������
 * WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ    
 * DataToWrite:Ҫд�������
 */
void CPLD_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{				   	  	    																 
    IIC_Start();  
    IIC_SendByte(0xB8); //����������ַ0XA0,д���� 
	IIC_WaitAck();	   
    IIC_SendByte(WriteAddr); //���͵͵�ַ
	IIC_WaitAck(); 	 										  		   
	IIC_SendByte(DataToWrite); 							   
	IIC_WaitAck();  		    	   
    IIC_Stop(); 
	delay_ms(10);	 
}

/*
 * ָ����ַ��ʼд�볤��ΪLen������
 * �ú�������д��16bit����32bit������.
 * WriteAddr  :��ʼд��ĵ�ַ  
 * DataToWrite:���������׵�ַ
 * Len        :Ҫд�����ݵĳ���2,4
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
 * ָ����ַ��ʼ��������ΪLen������
 * �ú������ڶ���16bit����32bit������.
 * ReadAddr   :��ʼ�����ĵ�ַ 
 * ����ֵ     :����
 * Len        :Ҫ�������ݵĳ���2,4
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
 * ���CPLD�Ƿ�����
 * ��������CPLD�����һ����ַ(255)���洢��־��.
 * ����1:���ʧ��
 * ����0:���ɹ�
 */
u8 CPLD_Check(void)
{
	u8 temp;
	temp = AT24CXX_ReadOneByte(255);//����ÿ�ο�����дAT24CXX
	if(temp == 0X55)
    {
        return 0;		   
    }
	else//�ų���һ�γ�ʼ�������
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
 * ��CPLD�����ָ����ַ��ʼ����ָ������������
 * ReadAddr :��ʼ�����ĵ�ַ ��24c02Ϊ0~255
 * pBuffer  :���������׵�ַ
 * NumToRead:Ҫ�������ݵĸ���
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
 * ��CPLD�����ָ����ַ��ʼд��ָ������������
 * WriteAddr :��ʼд��ĵ�ַ ��24c02Ϊ0~255
 * pBuffer   :���������׵�ַ
 * NumToWrite:Ҫд�����ݵĸ���
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
