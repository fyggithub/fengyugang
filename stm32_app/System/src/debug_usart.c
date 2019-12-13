#include "sys.h"
#include "stm32f10x_usart.h"
#include "debug_usart.h"	  
#include "usart.h"

#define DUSART_REC_LEN 200
#define BUFF_SIZE   64

u8 Debug_Buff[BUFF_SIZE] = {0};

/* 加入以下代码,支持printf函数,而不需要选择use MicroLIB	*/
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       

/* 定义_sys_exit()以避免使用半主机模式 */
_sys_exit(int x) 
{ 
	x = x; 
} 

/* 重定义fputc函数  */
int fputc(int ch, FILE *f)
{       
    /* 循环发送,直到发送完毕 */
	while((USART1->SR & 0X40) == 0);  
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8  DUSART_RX_BUF[DUSART_REC_LEN];
u16 DUSART_RX_STA = 0;       //接收状态标记	  
  
void duart_init(u32 baudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    USART_DeInit(USART1); //复位串口 1

    /* USART1_TX   GPIOA.9 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;  //复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART1_RX	  GPIOA.10初始化 */
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
    USART_InitStructure.USART_BaudRate            = baudRate;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
}

void Debug_String(u8 *pData,u16 len)
{
	u8 dbug_len = 0;
	dbug_len = len;

	memset(Debug_Buff,0,BUFF_SIZE);
	if(dbug_len > BUFF_SIZE - 2)
	{
		return;
	}
	
	memcpy(Debug_Buff,pData,dbug_len);
	Debug_Buff[dbug_len] = '\n';

	USART_SendString(USART1, Debug_Buff, dbug_len + 1);
}

void Debug_log_value(u8 *pData,u8 len,u16 val)
{
	u8 dbug_len = 0,value = 0;
	dbug_len = len;
	value = val;

	memset(Debug_Buff,0,BUFF_SIZE);
	if(dbug_len > BUFF_SIZE - 2)
	{
		return;
	}
	
	memcpy(Debug_Buff,pData,dbug_len);
	Debug_Buff[dbug_len] = value / 100 + '0';
	Debug_Buff[dbug_len + 1] = value % 100 /10 + '0';
	Debug_Buff[dbug_len + 2] = value % 10 + '0';
	Debug_Buff[dbug_len + 3] = '\n';
	
	USART_SendString(USART1, Debug_Buff, dbug_len + 4);
}

void USART1_IRQHandler(void)
{
	u8 Res;

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        Res = USART_ReceiveData(USART1);

        if((DUSART_RX_STA & 0x8000) == 0)//接收未完成
        {
            if(DUSART_RX_STA & 0x4000)//接收到了0x0d
            {
                if(Res != 0x0a)
                {
                    DUSART_RX_STA = 0;//接收错误,重新开始
                }
                else 
                {
                    DUSART_RX_STA |= 0x8000;	//接收完成了 
                }
            }
            else //还没收到0X0D
            {
                if(Res == 0x0d)
                {
                    DUSART_RX_STA |= 0x4000;
                }
                else
                {
                    DUSART_RX_BUF[DUSART_RX_STA & 0X3FFF] = Res;
                    DUSART_RX_STA++;
                    if(DUSART_RX_STA > (DUSART_REC_LEN - 1))
                    {
                        DUSART_RX_STA = 0;//接收数据错误,重新开始接收	  
                    }
                }
            }
        }
    }
	
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	if(USART_GetFlagStatus(USART1,USART_FLAG_ORE) == SET)
	{
		USART_ClearFlag(USART1,USART_FLAG_ORE);
		DUSART_RX_STA = 0;//接收数据错误,重新开始接收	  
	}
}
