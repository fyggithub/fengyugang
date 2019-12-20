#include "debug_usart.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"	  
#include "process.h"
#include "Buffer.h"
#include "Bits.h"

#define USART1_REC_LEN    USART_REC_LEN
#define USART3_REC_LEN    USART_REC_LEN
#define USART4_REC_LEN    USART_REC_LEN
#define USART5_REC_LEN    USART_REC_LEN


Cycbuff cycle_buff;

static u8 USART1_RX_BUF[USART1_REC_LEN];
static u8 USART3_RX_BUF[USART3_REC_LEN];
static u8 USART4_RX_BUF[USART4_REC_LEN] = {0};
static u8 USART5_RX_BUF[USART5_REC_LEN];

static u8 USART1_RX_FLAG = 0;       
static u8 USART3_RX_FLAG = 0;       
static u8 USART4_RX_FLAG = 0;      	  
static u8 USART5_RX_FLAG = 0;       

//static u16 USART1_RX_CNT = 0;         
static u16 USART3_RX_CNT = 0;         
static u16 USART4_RX_CNT = 0;       	  
static u16 USART5_RX_CNT = 0;        

 u8 USART_getRecvFlag(u8 usartx)
{
	u8 pBuff[] = "keydown_off";
    if(UART_CONTROL_3 == usartx)
    {
        return USART3_RX_FLAG;
	}
	else if(UART_CONTROL_4 == usartx)
	{
        return USART4_RX_FLAG;
	}
	else if(UART_CONTROL_5 == usartx)
	{
        return USART5_RX_FLAG;
	}
	else
	{
		Debug_String(pBuff,sizeof(pBuff) - 1);
		return 2;
	}
}

void USART_clearRecvFlag(u8 usartx)
{
	u8 pBuff[] = "unknown usart";
    if(UART_CONTROL_3 == usartx)
    {
        USART3_RX_FLAG = 0;
	}
	else if(UART_CONTROL_4 == usartx)
	{
        USART4_RX_FLAG = 0;
	}
	else if(UART_CONTROL_5 == usartx)
	{
        USART5_RX_FLAG = 0;
	}
	else 
	{
//        DebugPrint("unknown usart %x\n",usartx);
		Debug_String(pBuff,sizeof(pBuff) - 1);
	}
}

void USART_clearRecvLen(u8 usartx)
{
	u8 pBuff[] = "unknown usart";
    if(UART_CONTROL_3 == usartx)
    {
        USART3_RX_CNT = 0;
	}
	else if(UART_CONTROL_4 == usartx)
	{
        USART4_RX_CNT = 0;
	}
	else if(UART_CONTROL_5 == usartx)
	{
        USART5_RX_CNT = 0;
	}
	else 
	{
 //       DebugPrint("unknown usart %x\n",usartx);
		Debug_String(pBuff,sizeof(pBuff) - 1);
	}
}

void USART_getRecvData(u8 usartx, u8 *pData)
{	
	u8 pBuff[] = "unknown usart";
	
    if(UART_CONTROL_3 == usartx)
    {
        memcpy(pData, USART3_RX_BUF, USART3_REC_LEN);
	}
	else if(UART_CONTROL_4 == usartx)
	{
       memcpy(pData, USART4_RX_BUF, USART4_REC_LEN);
	}
	else if(UART_CONTROL_5 == usartx)
	{
        memcpy(pData, USART5_RX_BUF, USART5_REC_LEN);
	}
	else
	{
//        DebugPrint("unknown usart %x\n",usartx);
		Debug_String(pBuff,sizeof(pBuff) - 1);
	}
}

#if 0
void uart1_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
  
    /* USART1_TX GPIOA.9 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;  //复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART3_RX GPIOA.10初始化 */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Usart1 NVIC 配置 */
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* USART 初始化设置 */
    USART_InitStructure.USART_BaudRate            = bound;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
}

void USART1_IRQHandler(void)
{
	u8 Res;

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        Res = USART_ReceiveData(USART1);

        USART1_RX_BUF[USART1_RX_CNT] = Res;
        USART1_RX_CNT++;
        if(USART1_RX_CNT > (USART1_REC_LEN - 1))
        {
            USART1_RX_CNT = 0;
			USART1_RX_FLAG = 1;
        }
    }
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}
#endif

void uart3_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

	USART_DeInit(USART3); //复位串口 3
  
    /* USART3_TX GPIOA.9 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;  //复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* USART3_RX GPIOA.10初始化 */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Usart3 NVIC 配置 */
    NVIC_InitStructure.NVIC_IRQChannel                   = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* USART 初始化设置 */
    USART_InitStructure.USART_BaudRate            = bound;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART3, &USART_InitStructure);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART3, ENABLE);
}

void USART3_IRQHandler(void)
{
	u8 Res;

    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        Res = USART_ReceiveData(USART3);

        USART3_RX_BUF[USART3_RX_CNT] = Res;
        USART3_RX_CNT++;
        if(USART3_RX_CNT > (USART3_REC_LEN - 1))
        {
            USART3_RX_CNT = 0;
			USART3_RX_FLAG = 1;
        }
    }
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}

//初始化IO 串口4 
//pclk1CLK1时钟频率(Mhz) 
//bound:波特率	  
//void uart4_init(u32 pclk1,u32 bound) 
//{  	  
//	float temp; 
//	u16 mantissa; 
//	u16 fraction;	    
//	
//	temp = (float)(pclk1*1000000)/(bound*16);//得到USARTDIV 
//	mantissa = temp;	 //得到整数部分 
//	fraction = (temp-mantissa)*16; //得到小数部分	  
//	mantissa<<=4; 
//	mantissa += fraction;  

//	RCC->APB2ENR |= 1<<4;   	//使能PORTC口时钟   
//	RCC->APB1ENR |= 1<<19;  	//使能串口4时钟  
//	GPIOC->CRH &= 0XFFFF00FF;	//IO状态设置 
//	GPIOC->CRH |= 0X00008B00;	//IO状态设置 
//	GPIOC->ODR |= 1<<11;    	//rx上拉 
//	   
//	RCC->APB1RSTR |= 1<<19;   //复位串口4 
//	RCC->APB1RSTR &= ~(1<<19);//停止复位	   	    
//	//波特率设置 
//	UART4->BRR = mantissa; // 波特率设置	  
//	UART4->CR1 |= 0X200C;  //1位停止,无校验位.	  
//	//使能接收中断 
//	UART4->CR1 |= 1<<8;    //PE中断使能 
//	UART4->CR1 |= 1<<5;    //接收缓冲区非空中断使能	    	
//	MY_NVIC_Init(3,1,UART4_IRQn,2);//组2，抢占3，响应2， 
//}  

void UART4_Init_Set(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

	USART_DeInit(UART4); //复位串口 3
  
    /* UART4_TX GPIOA.10 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;  //复用推挽输出
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* UART4_RX GPIOC.11初始化 */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Usart4 NVIC 配置 */
    NVIC_InitStructure.NVIC_IRQChannel                   = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* USART 初始化设置 */
    USART_InitStructure.USART_BaudRate            = BAUD_115200;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(UART4, &USART_InitStructure);
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_Cmd(UART4, ENABLE);
}

void UART4_IRQHandler(void)
{
	u8 Res;
	u16 cmd = 0;
	
    if (USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
    {
        Res = USART_ReceiveData(UART4);
        USART4_RX_BUF[USART4_RX_CNT] = Res;
        USART4_RX_CNT++;
		
        if(USART4_RX_CNT > (USART4_REC_LEN - 1))
        {
            USART4_RX_CNT = 0;
			cmd = USART4_RX_BUF[3] * 256 + USART4_RX_BUF[2];	
			if(cmd > MAX_BOOT_CMD)
			{				
				USART4_RX_FLAG = 0;
				FastAck_AppCmd(USART4_RX_BUF,cmd);
			}
			else
			{	
				USART4_RX_FLAG = 1;
			}	
        }
    }
	USART_ClearITPendingBit(UART4, USART_IT_RXNE);
}

void UART5_IRQHandler(void)
{
	u8 Res;

    if (USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
    {
        Res = USART_ReceiveData(UART5);
        USART5_RX_BUF[USART5_RX_CNT] = Res;
        USART5_RX_CNT++;
        if(USART5_RX_CNT > (USART5_REC_LEN - 1))
        {
            USART5_RX_CNT = 0;
			USART5_RX_FLAG = 1;
        }
    }
	USART_ClearITPendingBit(UART5, USART_IT_RXNE);
}

s8 USART_ReadData(u8 usartx, u8 *pData, u16 len)
{
    u16 overTime   = 0;
    u16 recvLen    = 0;
    u8  *pRecvFlag = NULL;
    u8  *pRecvBuf  = NULL;
    
	if(UART_CONTROL_3 == usartx)
	{
        pRecvFlag = &USART3_RX_FLAG;
        pRecvBuf  = USART3_RX_BUF;
        recvLen   = USART3_REC_LEN;
	}
	else if(UART_CONTROL_4 == usartx)
	{
        pRecvFlag = &USART4_RX_FLAG;
        pRecvBuf  = USART4_RX_BUF;
        recvLen   = USART4_REC_LEN;
	}
	else if(UART_CONTROL_1 == usartx)
	{
        pRecvFlag = &USART1_RX_FLAG;
        pRecvBuf  = USART1_RX_BUF;
        recvLen   = USART1_REC_LEN;
	}
	else if(UART_CONTROL_5 == usartx)
	{
        pRecvFlag = &USART5_RX_FLAG;
        pRecvBuf  = USART5_RX_BUF;
        recvLen   = USART5_REC_LEN;
	}
	
	if(len > recvLen)	
	{
        return -1; 
	}
	
    while(overTime++ > 2000)
    {
        if(*pRecvFlag == 1)
        {
            *pRecvFlag = 0;
			memcpy(pData, pRecvBuf, len);
			return 0;
		}

		delay_ms(1);
	}

	return -1;
}

void USART_WriteData(u8 usartx, u8 *pData, u16 len)
{
	u8 pBuff[] = "unknown usart";
    if(UART_CONTROL_3 == usartx)
    {
        USART_SendString(USART3, pData, len);
	}
	else if(UART_CONTROL_4== usartx)
	{
		USART_SendString(UART4, pData, len);
	}
	else if(UART_CONTROL_5 == usartx)
	{
		USART_SendString(UART5, pData, len);
	}
	else if(UART_CONTROL_1 == usartx)
	{
		USART_SendString(USART1, pData, len);
	}
	else
	{
//        DebugPrint("unknown usart %x\n",usartx);
		Debug_String(pBuff,sizeof(pBuff) - 1);
	}
}

void Uart4_Buff_Init(void)
{
//    BufferCreate (&(cycle_buff.rxBuffer), cycle_buff.rxBufferArray, BUFF_MAX);		
}

void uart4_clean_buf(void)
{
	USART4_RX_CNT = 0;
	memset(USART4_RX_BUF, 0x00, sizeof(USART4_RX_BUF));	
}

