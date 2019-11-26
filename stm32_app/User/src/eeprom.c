#include "eeprom.h" 
#include "delay.h"
#include "usart.h"
#include "myiic.h"
#include "debug_usart.h"
/*
 * ��CPLDָ����ַ����һ������
 * ReadAddr:��ʼ�����ĵ�ַ  
 * ����ֵ  :����������
 */

#define AT24C02_LEN  (256)

u8 E2PROM_ReadOneByte(u8 ReadAddr)
{
	u8 temp = 0;

    IIC_Start(IIC_E_BUS0);
    IIC_SendByte(IIC_E_BUS0, 0xA0); //����������ַ0XA0,д����
	IIC_WaitAck(IIC_E_BUS0);
    IIC_SendByte(IIC_E_BUS0, ReadAddr); //���͵͵�ַ
	IIC_WaitAck(IIC_E_BUS0);
	IIC_Start(IIC_E_BUS0);
    IIC_SendByte(IIC_E_BUS0, 0xA1); //�������ģʽ
	IIC_WaitAck(IIC_E_BUS0);
    temp = IIC_ReadByte(IIC_E_BUS0, 0);
    IIC_Stop(IIC_E_BUS0);
	return temp;
}

/*
 * ָ����ַд��һ������
 * WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ    
 * DataToWrite:Ҫд�������
 */
void E2PROM_WriteOneByte(u8 WriteAddr, u8 DataToWrite)
{
    IIC_Start(IIC_E_BUS0);  
    IIC_SendByte(IIC_E_BUS0, 0xA0); //����������ַ0XA0,д���� 
	IIC_WaitAck(IIC_E_BUS0);	   
    IIC_SendByte(IIC_E_BUS0, WriteAddr); //���͵�ַ
	IIC_WaitAck(IIC_E_BUS0); 	 										  		   
	IIC_SendByte(IIC_E_BUS0, DataToWrite); 							   
	IIC_WaitAck(IIC_E_BUS0);  		    	   
    IIC_Stop(IIC_E_BUS0); 
	delay_ms(10);	 
}

/*
 * ָ����ַ��ʼд�볤��ΪLen������
 * WriteAddr  :��ʼд��ĵ�ַ  
 * DataToWrite:���������׵�ַ
 * Len        :Ҫд�����ݵĳ���
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
 * ָ����ַ��ʼ��������ΪLen������
 * ReadAddr   :��ʼ�����ĵ�ַ 
 * ����ֵ     :����
 * Len        :Ҫ�������ݵĳ���
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


