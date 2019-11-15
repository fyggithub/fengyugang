#include "debug_usart.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"	  
#include "process.h"
#include "debug_usart.h"

#define USART1_REC_LEN    USART_REC_LEN
#define USART3_REC_LEN    USART_REC_LEN

#define USART4_REC_LEN    USART_REC_LEN

static u8 USART1_RX_BUF[USART1_REC_LEN] = {0};
static u8 USART3_RX_BUF[USART3_REC_LEN] = {0};
static u8 USART4_RX_BUF[USART4_REC_LEN] = {0};

static u8 USART1_RX_FLAG = 0;       
static u8 USART3_RX_FLAG = 0;       
static u8 USART4_RX_FLAG = 0;      	  

static u16 USART1_RX_CNT = 0;         
static u16 USART3_RX_CNT = 0;         
static u16 USART4_RX_CNT = 0;       	  

 u8 USART_getRecvFlag(u8 usartx)
{
    if(UART_CONTROL_3 == usartx)
    {
        return USART3_RX_FLAG;
	}
	else if(UART_CONTROL_4 == usartx)
	{
        return USART4_RX_FLAG;
	}
	else if(UART_CONTROL_1 == usartx)
	{
        return USART1_RX_FLAG;
	}
	else
	{
		printf("boot unknown usart!\n");
        DebugPrint("unknown usart %x\n",usartx);
		return 2;
	}
}

void USART_clearRecvFlag(u8 usartx)
{
    if(UART_CONTROL_3 == usartx)
    {
        USART3_RX_FLAG = 0;
	}
	else if(UART_CONTROL_4 == usartx)
	{
        USART4_RX_FLAG = 0;
	}
	else if(UART_CONTROL_1 == usartx)
	{
        USART1_RX_FLAG = 0;
	}
	else 
	{
        DebugPrint("unknown usart %x\n",usartx);
	}
}

void USART_clearRecvLen(u8 usartx)
{
    if(UART_CONTROL_3 == usartx)
    {
        USART3_RX_CNT = 0;
	}
	else if(UART_CONTROL_4 == usartx)
	{
        USART4_RX_CNT = 0;
	}
	else if(UART_CONTROL_1 == usartx)
	{
        USART1_RX_CNT = 0;
	}
	else 
	{
        DebugPrint("unknown usart %x\n",usartx);
	}

}

void USART_getRecvData(u8 usartx, u8 *pData)
{
    if(UART_CONTROL_3 == usartx)
    {
        memcpy(pData, USART3_RX_BUF, USART3_REC_LEN);
	}
	else if(UART_CONTROL_4 == usartx)
	{
        memcpy(pData, USART4_RX_BUF, USART4_REC_LEN);      
	}
	else if(UART_CONTROL_1 == usartx)
	{
        memcpy(pData, USART1_RX_BUF, USART1_REC_LEN);
	}
	else
	{
        DebugPrint("unknown usart %x\n",usartx);
	}
}

void uart4_get_data(UART_CMD *pData)
{
	//printf("u=%d\n", USART4_RX_CNT);
	memset(pData, 0x00, sizeof(UART_CMD));
	memcpy(pData, (UART_CMD *)USART4_RX_BUF, USART4_REC_LEN);
}

#if 0
void uart1_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
  
    /* USART3_TX GPIOA.9 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART3_RX GPIOA.10��ʼ�� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Usart3 NVIC ���� */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* USART ��ʼ������ */
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
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
  
    /* USART3_TX GPIOA.9 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* USART3_RX GPIOA.10��ʼ�� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Usart3 NVIC ���� */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* USART ��ʼ������ */
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
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

void uart4_init(u32 bound)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    USART_DeInit(UART4);
  
    /* USART4_TX GPIOC.10 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;  //�����������
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* USART4_RX GPIOC.11��ʼ�� */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Usart4 NVIC ���� */
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* USART ��ʼ������ */
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(UART4, &USART_InitStructure);
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_Cmd(UART4, ENABLE);
}

void UART4_IRQHandler(void)
{
	u8 Res;

	//printf("yy \n");

    if (USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
    {
        Res = USART_ReceiveData(UART4);
        USART4_RX_BUF[USART4_RX_CNT] = Res;
        USART4_RX_CNT++;
        if(USART4_RX_CNT > (USART4_REC_LEN - 1))
        //if(USART4_RX_CNT > (USART4_REC_LEN))
        {
            USART4_RX_CNT = 0;
			USART4_RX_FLAG = 1;
        }
    }
	USART_ClearITPendingBit(UART4, USART_IT_RXNE);
	//printf("%d\n", Res);
}

s8 USART_ReadData(u8 usartx, u8 *pData, u16 len)
{
    u16 overTime = 0;
    u16 recvLen = 0;
    u8  *pRecvFlag;
	u8  *pRecvBuf;
    
	if(UART_CONTROL_3 == usartx)
	{
        pRecvFlag = &USART3_RX_FLAG;
		pRecvBuf = USART3_RX_BUF;
		recvLen = USART3_REC_LEN;
	}
	else if(UART_CONTROL_4== usartx)
	{
        pRecvFlag = &USART4_RX_FLAG;
		pRecvBuf = USART4_RX_BUF;
		recvLen = USART4_REC_LEN;
	}
	else if(UART_CONTROL_1 == usartx)
	{
        pRecvFlag = &USART1_RX_FLAG;
		pRecvBuf = USART1_RX_BUF;
		recvLen = USART1_REC_LEN;
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
    if(UART_CONTROL_3 == usartx)
    {
        USART_SendString(USART3, pData, len);
	}
	else if(UART_CONTROL_4 == usartx)
	{
		USART_SendString(UART4, pData, len);
	}
	else if(UART_CONTROL_1 == usartx)
	{
		USART_SendString(USART1, pData, len);
	}
	else
	{
        DebugPrint("unknown usart %x\n",usartx);
	}
}

void uart4_clean_buf(void)
{
	USART4_RX_CNT = 0;
	memset(USART4_RX_BUF, 0x00, sizeof(USART4_RX_BUF));	
}
