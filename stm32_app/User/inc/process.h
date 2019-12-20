#ifndef __PROCESS_H
#define __PROCESS_H
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

#define HEAD           (0xEBAA)

typedef enum
{
	APP_UPDATE_START_CMD = 0x0003,
	APP_UPDATE_CMD,
	APP_UPDATE_FINISH_CMD,
    FIRMWARE_VER_CMD,
	MAX_BOOT_CMD,
	
	LED_RED_CMD = 0x22,
	LED_GREEN_CMD,
	LED_BLUE_CMD ,
	
	MIC1_GAIN_CMD = 0x30,
	MIC2_GAIN_CMD,
	MIC1_ENABLE_CMD,
	MIC2_ENABLE_CMD,
	
	OLED_CLEAR_CMD = 0x47,
	OLED_LOGO_CMD  = 0x48,
	OLED_LOGO_UPDATE_CMD = 0x2A,
	OLED_IP_CMD = 0x25,
	OLED_CTL_CMD = 0x2E,
	OLED_STRINGS_CMD = 0x2D,
	
	FAN_AUTO_CMD = 0x7B,
	FAN_SET_SPEED_CMD = 0x7C,
	READ_TEMPERATURE_CMD = 0x7D,
	
	SYS_KEY_SHUTDOWN_CMD = 0x61,
	SYS_POWER_CMD = 0x62,
	SYS_WATCH_KEY_SHUTDOWN_CMD = 0x63,
	
	IR_CODE_VALUE = 0x70,
	SYS_IR_SHUTDOWN_CMD = 0x71
}PROC_CMD;

typedef enum
{
    STM32_RES_OK = 0,
    STM32_RES_UNKNOWN,
    STM32_RES_FAIL,   
    STM32_RES_DATA_ERR,
}Respond;

#define FLASH_ADDR_APP 	       0x08010000  //App程序存放地址
#define FLASH_ADDR_BOOT        0x08000000
#define FLASH_ADDR_FLAG 	   0x08070000  // 升级标记的地址

void Send_To_Request(PROC_CMD cmd);
void SendTo_Vlaue(PROC_CMD cmd,u8 val);
void SendTo_Vlaue2(PROC_CMD cmd,u8 val,u8 val2);
void AppCmd_Fun(UART_CMD *pData);
void FastAck_AppCmd(u8 *send_buf,u16 cmd);
extern void hostBoardProc(void);
extern void sysBoardProc(void);
extern void Proc_setVer(void);

extern int g_cmd;

#endif
