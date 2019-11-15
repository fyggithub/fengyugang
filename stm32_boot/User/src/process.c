#include "debug_usart.h"
#include "sys.h"
#include "flash.h"
#include "process.h"
#include "usart.h"
#include "delay.h"
#include "gpio.h"

static UART_CMD stUartCMD         = {0x0};
static UART_CMD stResCMD          = {0x0};
static u32      g_offsetAddr      = FLASH_ADDR_APP;
static u8       g_recvDataErr     = 0;
static u8       s_flashData[2048] = {0x0,};
static u8       s_saveData[2]     = {0x0};
u32             cnt               = 0;
u8              g_softVer         = 0;

void Proc_setVer(u8 val)
{
    g_softVer = val;
}

u8 Proc_getVer(void)
{
    return g_softVer;
}

void Proc_ClrDataCnt(void)
{
    cnt = 0;
    DebugPrint("Proc_ClrDataCnt");
}

static u8 Proc_CheckSum(u8 *pData, u16 len)
{
    u16 i   = 0;
    u8  val = 0;
	
	for(i = 0; i < len; i++)
	{
        val += pData[i];
	}

	return val;
}

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

static void Proc_Stm32Respond(u8 usartx, UART_CMD *pData, UART_CMD *pRes, u16 resDataLen)
{
    u8 checkSum;
	
	pRes->head = HEAD;
	pRes->cmd  = pData->cmd;
	pRes->dataLen = resDataLen;

	checkSum = Proc_CheckSum((u8 *)pRes, sizeof(UART_CMD) - 1); //减去校验位
	pRes->cs = 0 - checkSum;
	USART_WriteData(usartx, (u8 *)pRes, sizeof(UART_CMD));
}

/*APP程序数据写入flash
  pData:每包大小固定2K字节,
  len  :数据包长固定2048
*/
static s8 Proc_WriteAppFlash(u8 *pData, u16 len)
{
	STMFLASH_Write(g_offsetAddr, (u16 *)pData, len / 2);
	/* g_offsetAddr += FLASH_SECTOR_SIZE; */
    /* DebugPrint("g_offsetAddr=0x%x\n", g_offsetAddr); */
	g_offsetAddr += len;

	return 0;
}

static void Proc_Finished(void)
{
    DebugPrint("before load app\n");
    Iap_Load_App(FLASH_ADDR_APP);
}

static void Proc_DataErrProc(u8 usartx)
{
    if(g_recvDataErr == 1)
    {
        g_recvDataErr = 0;
		USART_clearRecvLen(usartx);
	}
}

static void Proc_UsartContorl(u8 usartx)
{
	UART_CMD *pUartCMD   = &stUartCMD;
	UART_CMD *pResCMD    = &stResCMD;
	s8        ret        = 0;
	u8        checkSum   = 0;
	u16       resDataLen = 0;
    u16       data       = 0;
	static int flag_value = 0;
    /* u16 i = 0; */

    /* 串口是否有接收到数据 */
	if(USART_getRecvFlag(usartx) == 1)
	{
        USART_clearRecvFlag(usartx);
        memset(pUartCMD, 0x00, sizeof(UART_CMD));
		USART_getRecvData(usartx, (u8 *)pUartCMD);

		checkSum = Proc_CheckSum((u8 *)pUartCMD, sizeof(UART_CMD));
        if(0 != checkSum)
        {
            DebugPrint("checkSum=%d, cmd1=%d \n", checkSum, pUartCMD->cmd);           
        }
		if((checkSum == 0) && (pUartCMD->head == HEAD))	
		{
		    switch(pUartCMD->cmd)
		    {
                case APP_UPDATE_START_CMD:
                    DebugPrint("APP_UPDATE_START_CMD\n");
					g_offsetAddr = FLASH_ADDR_APP;
                    ret = 0;
                    resDataLen = 0;
                    cnt = 0;
					break;

				case APP_UPDATE_CMD:
                    if (0 == cnt)
                    {
                        memset(s_flashData, 0x00, 2048);
                    }
                    /* DebugPrint("APP_UPDATE_CMD\n"); */
                    memcpy(&s_flashData[16*cnt], pUartCMD->data, pUartCMD->dataLen);
                    if(FLASH_ADDR_APP == g_offsetAddr && 0 == cnt)
                    {
                        //DebugPrint("data[0]=0x%x data[1]=0x%x\n", s_flashData[0], s_flashData[1]);
                        memset(s_saveData, 0x0, sizeof(s_saveData));
                        s_saveData[0] = s_flashData[0];
                        s_saveData[1] = s_flashData[1];
                        s_flashData[0] = 0xff;
                        s_flashData[1] = 0xff;
                    }

                    cnt++;
                    if (128 == cnt)
                    {
                        ret = Proc_WriteAppFlash(s_flashData, 2048);
                        cnt = 0;
                    }

                    resDataLen = 0;
					break;

				case APP_UPDATE_FINISH_CMD:
                    DebugPrint("APP_UPDATE_FINISH_CMD cnt=%d\n", cnt);
                    ret = Proc_WriteAppFlash(s_flashData, cnt*16);
                    STMFLASH_Write(FLASH_ADDR_APP, (u16 *)s_saveData, 0x1);
                    STMFLASH_Read(FLASH_ADDR_APP, &data, 0x1);
                    DebugPrint("data=0x%x\n", data);
                    delay_ms(100);
                    ret = 0;
                    resDataLen = 0;
                    Proc_setRespondState(pResCMD, ret);
                    Proc_Stm32Respond(usartx, pUartCMD, pResCMD, resDataLen);
                    delay_ms(100);
                    Proc_Finished();
					flag_value++;
					printf("flag = %d\n",flag_value);
					break;

				case FIRMWARE_VER_CMD:
                    ret = 0;
                    pResCMD->data[0] = Proc_getVer();
                    DebugPrint("ver:0x%x\n", pResCMD->data[0]);
                    resDataLen = 1;
					break;


				default:
					ret = 1;
					break;					
			}
		}
		else //接收数据错误
		{
            ret = 2;
			g_recvDataErr = 1;
		}

		Proc_setRespondState(pResCMD, ret);
		Proc_Stm32Respond(usartx, pUartCMD, pResCMD, resDataLen);

		memset(pUartCMD, 0, sizeof(UART_CMD));
		memset(pResCMD, 0, sizeof(UART_CMD));
    }

	if(flag_value == 1)
	{
		printf("flag = %d,jump_addr fail!\n",flag_value);
	}
	Proc_DataErrProc(usartx);
}

void Proc_appUpdate(void)
{
    Proc_UsartContorl(UART_CONTROL_4);
}
