#ifndef __EEPROM_H
#define __EEPROM_H

#include "stm32f10x.h"

extern u8 E2PROM_ReadOneByte(u8 ReadAddr);
extern void E2PROM_WriteOneByte(u8 WriteAddr, u8 DataToWrite);
extern void E2PROM_WriteLenByte(u8 WriteAddr, u8 *pBuf, u8 Len);
extern u8 E2PROM_ReadLenByte(u8 ReadAddr, u8 *pBuf, u8 Len);
extern int bsp_e2promWrte(u8 *pSrcData);
extern u8 bsp_e2promRead(u8 *pSrcData, u8 *pResData, u16 *resDataLen);
#endif
















