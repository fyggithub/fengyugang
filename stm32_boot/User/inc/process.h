#ifndef __PROCESS_H_
#define __PROCESS_H_

#include "stm32f10x.h"

/*定义串口数据协议各字段长度*/
#define UART_HEAD_LEN      2
#define UART_CMD_LEN       2
#define UART_DATALEN_LEN   2
#define UART_DATA_LEN      16

#define UART_STATE_LEN     1
#define UART_CS_LEN        1  

/*串口数据协议定义*/
typedef struct tagsUART_CMD
{
    u16 head;
    u16 cmd;
    u8  data[UART_DATA_LEN];
	u16 dataLen;
    u8  state;
    u8  cs;
}UART_CMD;

/*定义最大接收字节数*/
#define USART_REC_LEN  	(UART_HEAD_LEN + UART_CMD_LEN + UART_DATALEN_LEN + \
	                     UART_DATA_LEN + UART_STATE_LEN + UART_CS_LEN)

#define HEAD            (0xEBAA)

typedef enum
{
	APP_UPDATE_START_CMD = 0x0003,
	APP_UPDATE_CMD,
	APP_UPDATE_FINISH_CMD,
    FIRMWARE_VER_CMD,
}PROC_CMD;

typedef enum
{
    STM32_RES_OK = 0,
    STM32_RES_UNKNOWN,
    STM32_RES_FAIL,   
    STM32_RES_DATA_ERR,
}Respond;

/*
 * typedef struct tagsUART_Respond_CMD
 * {
 *     u16 head;
 *     u16 cmd;
 *     u8  state;
 *     u8  cs;
 * }UART_Respond_CMD;
 */

#define FLASH_ADDR_FLAG 	   0x08070000  //存放标志位 0x55aa运行下载程序，0 运行APP
#define FLASH_PAGE_OF_APP       32
#define FLASH_ADDR_APP 	        0x08010000  //App程序存放地址
#define FLASH_SECTOR_SIZE       (2048)

#define UPDATE_FLAG             0x55aa
#define APP_FLAG                0

extern void Proc_appUpdate(void);
extern void Proc_ClrDataCnt(void);
extern void Proc_setVer(u8 val);

#endif
