#ifndef __DEBUG_USART_H
#define __DEBUG_USART_H

#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"

#define DebugPrint(fmt,...)  printf("%s(%d)-<%s>:"fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__) 

void Debug_String(u8 *pData,u16 len);
void Debug_log_value(u8 *pData,u8 len,u16 val);
void Debug_Value_Hex(u8 *pData,u8 len,u16 val);
extern void duart_init(u32 bound);

#endif
