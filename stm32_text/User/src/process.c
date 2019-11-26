#include "sys.h"
#include "debug_usart.h"
#include "process.h"
#include "usart.h"
#include "flash.h"
#include "debug_usart.h"
#include "delay.h"
#include "stm32f10x_usart.h"
#include "myiic.h"

static UART_CMD stUartCMD;
static UART_CMD stResCMD;
static u8       g_recvDataErr = 0;
u8              g_softVer     = 0;
extern unsigned char reg_val[I2C_REG_NUM];

void Proc_setVer(void)
{
    reg_val[SW_VERSION] = 0x0a; 
    reg_val[HW_VERSION] = 0x03;
}

u8 Proc_getVer(void)
{
    return g_softVer;
}

static u8 Proc_goToBoot(void)
{
    //u16 updateFlag = 0xffff;
    u16 updateFlag = 0x55aa;

	//STMFLASH_Write(FLASH_ADDR_APP+4, &updateFlag, 0x2);
	STMFLASH_Write(FLASH_ADDR_FLAG+4, &updateFlag, 0x1);
	Iap_Load_App(FLASH_ADDR_BOOT);

	return 0;
}

static u8 Proc_CheckSum(u8 *pData, u16 len)
{
    u16 i;
    u8 val = 0;
	
	for(i = 0; i < len; i++)
	{
        val +=pData[i];
	}

	return val;
}

static void Proc_Stm32Send(u8 usartx, u8 cmd, UART_CMD *pRes, u16 resDataLen)
{
    u8 checkSum;
	
	pRes->head = HEAD;
	pRes->cmd  = cmd;
	pRes->dataLen = resDataLen;

	checkSum = Proc_CheckSum((u8 *)pRes, sizeof(UART_CMD) - 1);
	pRes->cs = 0 - checkSum;

    USART_WriteData(usartx, (u8 *)pRes, sizeof(UART_CMD));
}

static void Proc_Stm32Respond(u8 usartx, UART_CMD *pData, UART_CMD *pRes, u16 resDataLen)
{
    u8 checkSum;
	
	pRes->head = HEAD;
	pRes->cmd  = pData->cmd;
	pRes->dataLen = resDataLen;

	checkSum = Proc_CheckSum((u8 *)pRes, sizeof(UART_CMD) - 1);
	pRes->cs = 0 - checkSum;
    /*
     * DebugPrint("checkSum=%d pRes->cs=%d head=0x%x cmd=%d dataLen=%d state=%d \n", 
     *         checkSum, pRes->cs, pRes->head, pRes->cmd, pRes->dataLen, pRes->state);
     */

    USART_WriteData(usartx, (u8 *)pRes, sizeof(UART_CMD));
}

#if 1
static void Proc_setRespondState(UART_CMD *pRes, s8 state)
{
    if(0 == state)
    {
	    pRes->state = STM32_RES_OK;
	}
    else if(state == -1)
	{
		pRes->state = STM32_RES_FAIL; 
	}
	else if(state == 1)
	{
		pRes->state = STM32_RES_UNKNOWN;
	}
	else if(state == 2)
	{
        pRes->state = STM32_RES_DATA_ERR;
	}

}
#endif

void Send_To_Request(PROC_CMD cmd)
{
	UART_CMD *pSend = &stUartCMD;

	memset(pSend, 0x0, sizeof(UART_CMD));
	pSend->cmd = cmd;		
	Proc_Stm32Send(UART_CONTROL_4, cmd, pSend, UART_DATA_LEN);
}

void SendTo_IrKeyVlaue(PROC_CMD cmd,u8 val)
{
	UART_CMD *pSend = &stUartCMD;

	memset(pSend, 0x0, sizeof(UART_CMD));
	pSend->cmd = cmd;
	pSend->data[0] = val; 
	Proc_Stm32Send(UART_CONTROL_4, cmd, pSend, UART_DATA_LEN);
}

static void Proc_DataErrProc(u8 usartx)
{
    if(g_recvDataErr == 1)
    {
        g_recvDataErr = 0;
		USART_clearRecvLen(usartx);
	}
}

/* -----------------------------------------------------------------
*  | 包头 | 命令字 | 数据长度 |     数据        | 状态码 | 校验和 | 
*  -----------------------------------------------------------------
*  | 2Byte| 2Byte  | 2Byte    |     16Byte      |  1Byte | 1Byte  |
*  -----------------------------------------------------------------
*/   
static void Proc_UartControl(u8 usartx)
{
    UART_CMD *pUartCMD = &stUartCMD;
    UART_CMD *pResCMD  = &stResCMD;

//	int i;
	s8  ret        = 0;
	u8  checkSum   = 0;
	u16 resDataLen = 0;
		
	if(USART_getRecvFlag(usartx) == 1)
	{
        memset(pUartCMD, 0x0, sizeof(UART_CMD));
        memset(pResCMD, 0x0, sizeof(UART_CMD));

        USART_clearRecvFlag(usartx);
		USART_getRecvData(usartx, (u8 *)pUartCMD);

		checkSum = Proc_CheckSum((u8 *)pUartCMD, sizeof(UART_CMD));
		if((checkSum == 0) && (pUartCMD->head == HEAD) && (pUartCMD->dataLen <= UART_DATA_LEN))
		{
			if(pUartCMD->cmd < MAX_BOOT_CMD)
			{
				printf("checkSum=%d, cmd=0x%x \n", checkSum, pUartCMD->cmd); 
				switch(pUartCMD->cmd)
				{           
					case APP_UPDATE_START_CMD:
								{
									ret = 0;
									Proc_setRespondState(pResCMD, ret);
									Proc_Stm32Respond(usartx, pUartCMD, pResCMD, resDataLen);
									printf("reboot for upgrade\n");
									Proc_goToBoot();                        //跳转到bootloader起始地址
								}break;													
					case FIRMWARE_VER_CMD: 									//程序版本查询
								{
									pResCMD->data[0] = Proc_getVer();
									ret = 0;
									resDataLen = 0x1;
									DebugPrint("ver:0x%x\n", pResCMD->data[0]);
								}break;
					default:pResCMD->state = STM32_RES_UNKNOWN;break;					
				}
			}
			else
			{
				printf("checkSum=%d, cmd=0x%x \n", checkSum, pUartCMD->cmd);      
				AppCmd_Fun(pUartCMD); 
			}
		}
		else
		{
            g_recvDataErr = 1;
			pResCMD->state = STM32_RES_DATA_ERR;
		}
        Proc_setRespondState(pResCMD, ret);
		Proc_Stm32Respond(usartx, pUartCMD, pResCMD, resDataLen);
	}
    Proc_DataErrProc(usartx);
}

/*主控板处理*/
void hostBoardProc(void)
{
	Proc_UartControl(UART_CONTROL_4);	// AM5728与单片机间的串口，量产后可用于升级单片机
}

/*串口命令接口函数*/
void AppCmd_Fun(UART_CMD *pData)
{
	int i,len = 0; 
	switch(pData->cmd)
	{		         							
		case LED_RED_CMD:   
					{
						reg_val[RED_LED_CTL] = pData->data[0];   
						if(reg_val[RED_LED_CTL] == 0x24)
						{
							reg_val[LED_COUNT_R] = 2;
						}
					}break;										
		case LED_GREEN_CMD:
					{
						reg_val[GREEN_LED_CTL] = pData->data[0];
						if(reg_val[GREEN_LED_CTL] == 0x24)
						{
							reg_val[LED_COUNT_G] = 2;
						}
					}break;			 
		case LED_BLUE_CMD:
					{
						reg_val[BLUE_LED_CTL] = pData->data[0];
						if(reg_val[BLUE_LED_CTL] == 0x24)
						{
							reg_val[LED_COUNT_B] = 2;
						}
					}break;			  		
		case OLED_LOGO_CMD:                                             //LOGO显示
					{
						reg_val[OLED_Logo] = pData->data[0];
						len = reg_val[OLED_Logo] & 0x7f;
						if(len < 16)
						{
							for(i = 0; i < len ; i++)
							{
								reg_val[i + OLED_Logo_1] = pData->data[1 + i];
							}
						}
						else
						{
							printf("send data len error!\n");
						}					
					}break;				
		case OLED_CLEAR_CMD:reg_val[SYS_INIT_COM] = pData->data[0];break; //OLED清屏
		case OLED_LOGO_UPDATE_CMD:                                        //改变图案
				reg_val[OLED_REQ_PIC] = pData->data[0];
				reg_val[OLED_PIC] = pData->data[1];
				break;
		case OLED_IP_CMD:                                                //IP显示
				for(i = 0;i < 5;i++)
				{
					reg_val[OLED_IP + i] = pData->data[i];
				}
				break;
		case OLED_CTL_CMD:                                               //OLED开关控制
				{
					reg_val[OLED_DISPLAY_OFF] = pData->data[0];
					if(reg_val[OLED_DISPLAY_OFF] & 0x04)
					{
						reg_val[OLED_CONTRAST] = pData->data[1];
					}
				}
				break;
		case OLED_STRINGS_CMD:                                          //OLED字符串显示
				{
					reg_val[OLED_ASC] = pData->data[0];
					len = reg_val[OLED_ASC] & 0x7f;
					if(len < 16)
					{
						memcpy(reg_val+OLED_1,pData->data+1,len);
					}
					else
					{
						printf("send data len error!\n");
					}
				}break;		
		case FAN_AUTO_CMD:      reg_val[SYS_FAN_AUTO_CTRL_DISABLE] = pData->data[0];break;   //风扇自动控制
		case FAN_SET_SPEED_CMD: reg_val[SYS_CTL_FAN] = pData->data[0]; 
								reg_val[SYS_FAN_SPEED] = pData->data[1];             
								break;   //风扇速度控制
		case SYS_KEY_SHUTDOWN_CMD:  reg_val[REQ_SHUTDOWN] = pData->data[0];         break;   //系统关机
		case SYS_IR_SHUTDOWN_CMD:   reg_val[REQ_SHUTDOWN] = pData->data[0];         break;   //系统关机
		case SYS_POWER_CMD:     reg_val[REBOOT_REG] = pData->data[0];               break;   //系统开机
		default:break;
	}
}




