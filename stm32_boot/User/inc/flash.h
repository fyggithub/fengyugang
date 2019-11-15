#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f10x.h" 


//������ѡ���оƬ������
#define STM32_FLASH_SIZE 512 	 			//RCT6��ѡSTM32��FLASH������С(��λΪK)
//#define STM32_FLASH_SIZE        128 	 	//RBT6 ��ѡSTM32��FLASH������С(��λΪK)
#define STM32_FLASH_WREN        1           //ʹ��FLASHд��(0��������;1��ʹ��)
#define STM32_FLASH_BASE       0x08000000 	//STM32 FLASH����ʼ��ַ

 
u16  STMFLASH_ReadHalfWord(u32 faddr);  //��������  
void STMFLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	//ָ����ַ��ʼд��ָ�����ȵ�����
u32  STMFLASH_ReadLenByte(u32 ReadAddr,u16 Len);						//ָ����ַ��ʼ��ȡָ����������
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����
void bsp_flashEraseStartPage(u8 startPage);

//����д��
void STMFLASH_WtiteU16DataToFlash(u32 WriteAddr,u16 WriteData);								   
#endif


















