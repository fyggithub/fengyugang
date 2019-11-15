#ifndef __USART_H
#define __USART_H
#include "stm32f10x.h"
#include "process.h"

#define UART_CONTROL_1   4
#define UART_CONTROL_3   1
#define UART_CONTROL_4   2 
#define UART_CONTROL_5   3

extern s8   USART_ReadData(u8 usartx, u8 *pData, u16 len);
extern void USART_WriteData(u8 usartx, u8 *pData, u16 len);
extern u8   USART_getRecvFlag(u8 usartx);
extern void USART_clearRecvFlag(u8 usartx);
extern void USART_clearRecvLen(u8 usart);
extern void USART_getRecvData(u8 usartx, u8 *pData);
extern void uart4_get_data(UART_CMD *pData);
extern void uart4_clean_buf(void);

extern void uart1_init(u32 bound);
extern void uart3_init(u32 bound);
extern void uart4_init(u32 bound);
extern void uart5_init(u32 bound);

#endif
