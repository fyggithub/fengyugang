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
        if(0 != checkSum)
        {
            //DebugPrint("head:%x cmd:%x dataLen:%d\n", pUartCMD->head, 
            //        pUartCMD->cmd, pUartCMD->dataLen);
            
            /*
            for(i = 0; i < pUartCMD->dataLen; i++)
            {
                printf("%x ",pUartCMD->data[i]);
            }
            printf("\n");
            */
            printf("checkSum=%d, cmd=%d \n", checkSum, pUartCMD->cmd);            
        }

		if((checkSum == 0) && (pUartCMD->head == HEAD) && (pUartCMD->dataLen <= UART_DATA_LEN))
		{
            DebugPrint("cmd=%d\n", pUartCMD->cmd);
            switch(pUartCMD->cmd)
            {           
                case APP_UPDATE_START_CMD:
                    ret = 0;
                    Proc_setRespondState(pResCMD, ret);
                    Proc_Stm32Respond(usartx, pUartCMD, pResCMD, resDataLen);
                    printf("reboot for upgrade\n");
                    Proc_goToBoot();
                    break;

                case FIRMWARE_VER_CMD:
                    pResCMD->data[0] = Proc_getVer();
                    ret = 0;
                    resDataLen = 0x1;
                    DebugPrint("ver:0x%x\n", pResCMD->data[0]);
                    break;

                default:
                    pResCMD->state = STM32_RES_UNKNOWN;
                    break;
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
 
