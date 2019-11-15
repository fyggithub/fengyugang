#ifndef __CPLD_H
#define __CPLD_H

#include "stm32f10x.h"

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767  
#define EE_TYPE AT24C02
					  
extern u8   CPLD_ReadOneByte(u16 ReadAddr);
extern void CPLD_WriteOneByte(u16 WriteAddr,u8 DataToWrite);
extern void CPLD_WriteLenByte(u16 WriteAddr, u32 DataToWrite, u8 Len);
extern u32  CPLD_ReadLenByte(u16 ReadAddr, u8 Len);
extern void CPLD_Write(u16 WriteAddr, u8 *pBuffer, u16 NumToWrite);
extern void CPLD_Read(u16 ReadAddr, u8 *pBuffer, u16 NumToRead);

extern u8   CPLD_Check(void); //检查器件
extern void CPLD_Init(void);    //初始化IIC


extern s8 CPLD_ReadProc(u8 *pData, u16 len, u8 *pResData, u16 *resDataLen);
extern s8 CPLD_WriteProc(u8 *pData, u16 len);

#endif
















