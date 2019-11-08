#include "sys.h"
#include "stm32f10x_gpio.h"
#include "debug_usart.h"
#include "ir.h"
#include "myiic.h"
#include "delay.h"
#include "tmds181.h"
#include "gpio.h"
#include "led.h"

#define GET_DATA_NUM		33		// 接收到数据个数，收到33个数据，包括32位数和以一个同步头
#define GPIOA4_DATA			GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4)	// CAMERA2_UART_IR
#define GPIOA5_DATA			GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5)	// CAMERA1_UART_IR
#define GPIOD11_DATA		GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11)	// FRP_IRIN
#define GPIOA6_DATA		    GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)	// CAMERA1_VISCA_IR
#define GPIOA7_DATA		    GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7)	// CAMERA2_HDBT_IR
#define PRESCALER_VALUE		99		/* 设置自动重装载寄存器周期的值 */
#define PERIOD_VALUE		71		/* 设置时钟频率除数的预分频值 */

#define SYS_CODE_HIGH_MIN		41		/* 同步码高电平持续最小时间 */
#define SYS_CODE_HIGH_MAX		48		/* 同步码高电平持续最大时间 */
#define LOGIC_1_HIGN_MIN		14		/* 逻辑1高电平持续最小时间 */
#define LOGIC_1_HIGN_MAX		18		/* 逻辑1高电平持续最大时间 */
#define LOGIC_0_HIGH_MIN		3		/* 逻辑0高电平持续最小时间 */
#define LOGIC_0_HIGH_MAX		8		/* 逻辑0高电平持续最大时间 */
#define REPEAT_CODE_HIGH_MIN	20		/* 重复码高电平持续最小时间 */
#define REPEAT_CODE_HIGH_MAX	25	/* 重复码高电平持续最大时间 */
#define RISING_FLAG				0x10	/* 标记上升沿被捕获 */
#define SYS_CODE_FLAG			0x80	/* 接收到引导码标识 */
#define RECEIVE_OK				0x40	/* 接收完成 */
#define IRUSING_CAM1_HDBT			0x01	/* CAMERA1_HDBT_IR(CAMERA1_UART_IR)正在接收红外标志 */
#define IRUSING_CAM2_VISCA			0x02	/* CAMERA2_VISCA_IR(CAMERA2_UART_IR)正在接收红外标志 */
#define IRUSING_FRP				0x04	/* FRP_IRIN正在接收红外标志 */
#define IRUSING_CAM1_VISCA      0x08    /* CAMERA1_VISCA_IR 正在接收红外标志 */
#define IRUSING_CAM2_HDBT       0x10    /* CAMERA2_HDBT_IR 正在接收红外标志 */
#define IRUSING_FLAG			0xff	/* 标志位，从低到高分别代表: cam1,cam2, FRP_IRIN */
#define IR_PERIOD				900		/* 红外接收数据的周期时间，单位0.1ms */

#define IR_NOISE                200     /* 红外电源干扰周期在20ms以内 */ 
#define IR_NOISE_MIN            85      /* 同步码最小时间 */ 
#define IR_NOISE_MAX            95      /* 同步码最大时间 */ 

#define IRQ_ON  1
#define IRQ_OFF 0 
/* [7]: 收到引导码标志 */

u8	irSta = 0;					// 接收器的状态
u32 rmRec = 0;					// 红外接收到的数据
u8  keyCnt=0;					// 一直按，按键按下的次数
u16	ucTime2Flag = 0;			// 两次外部中断之间的时间
u16	ucTime1Flag = 0;			// 两次外部中断之间的时间
u8 sysflag_S = 0;               // 起始码开始标志

u8	idx = 0;					// 索引接收到的数值
u8 irUsing = 0;					// 0x01:CAMERA1_UART_IR; 0x02:CAMERA2_UART_IR; 0x04:FRP_IRIN
u32 irq_timecnt = 0;            // 每300ms重新开启中断

u8 camera_ir_flag = 0;          // 0x01:camera1低电平标记，0x02:camera2低电平标记，0x10:camera1噪声标记，0x20:camera2噪声标记
int irq_flag = IRQ_ON;

//extern unsigned int cur_time;
extern unsigned int camera1_time;   /* camera1当前时间 */
extern unsigned int camera2_time;   /* camera2当前时间 */
extern unsigned int s_numof1s;      /* 秒记时2^31s */
extern unsigned char IR_NOISE_DETECT; /* ir中断屏蔽时间 */
extern unsigned char reg_val[I2C_REG_NUM]; 

static void ir_receive_data(void);

int on_off	= 1;	// 5728上下电控制

static void ir_exti_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* FRP_IRIN */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;   
	//GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* CAMERA2_VISCA_IR, CAMERA1_HDBT_IR, CAMERA1_VISCA_IR */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

//	/* 输出到PK3399 */
//	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_3 | GPIO_Pin_2;
//	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);	

//    /* CAM1_IR_OUT CAM2_IR_OUT */
//	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_3 | GPIO_Pin_4;
//	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
//	GPIO_Init(GPIOD, &GPIO_InitStructure);	    
    
    /* CAM1_IR_OUT CAM2_IR_OUT */
	EXTI_ClearITPendingBit(EXTI_Line5);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource5);
	EXTI_InitStructure.EXTI_Line 	= EXTI_Line5;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* CAMERA1_VISCA_IR */
	EXTI_ClearITPendingBit(EXTI_Line6);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource6);
	EXTI_InitStructure.EXTI_Line 	= EXTI_Line6;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

    /* CAMERA2_HDBT_IR PA7 */
	EXTI_ClearITPendingBit(EXTI_Line7);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7);
	EXTI_InitStructure.EXTI_Line 	= EXTI_Line7;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);		

	// CAMERA2_UART_IR
	EXTI_ClearITPendingBit(EXTI_Line4);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource4);
	EXTI_InitStructure.EXTI_Line 		= EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger 	= EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd 	= ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// FRP_IRIN 
	EXTI_ClearITPendingBit(EXTI_Line11);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource11);
	EXTI_InitStructure.EXTI_Line 	= EXTI_Line11;
	EXTI_InitStructure.EXTI_Mode	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	//EXTI_GenerateSWInterrupt(EXTI_Line11);	//软件自动触发，继而转入中断处理函数

	/* set the Vector Table base location at 0x20000000 */
	//NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
	/* set the Vector Table base location at 0x80000000 */
	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);		

/*
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);		    
*/

    #if 0
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);		    
    #endif
}

/**
 * Tout = (prescaler+1)*(period+1) / Tclk
 * Tout: 中断时间，Tclk：输入的时钟，不分频72MHz
 * 这里设置为0.1ms   prescaler: 100-1, period: 72-1
 */
static void ir_tim2_init(u16 prescaler, u16 period)
{
	TIM_TimeBaseInitTypeDef TIM_StructInit;
	NVIC_InitTypeDef NVIC_StructInit;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_StructInit.TIM_Period = period;
	TIM_StructInit.TIM_Prescaler = prescaler;
	TIM_StructInit.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_StructInit.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_StructInit.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_StructInit);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);

	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);	// 待确认
	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x10000);	// 待确认
	NVIC_StructInit.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_StructInit.NVIC_IRQChannelCmd = ENABLE;
	NVIC_StructInit.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_StructInit.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_StructInit);
}
/* 接收判断条件：IR都没有接收数据，或者自己正在接收 */
void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4)!=RESET)
    {
        if (!(irUsing & IRUSING_FLAG) || (irUsing & IRUSING_CAM2_VISCA))
        {
            if (IRQ_ON == irq_flag)
            {
                EXTI_ClearITPendingBit(EXTI_Line5); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line6); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line7); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line11); // 清除中断标志
                EXTI->IMR &= ~(EXTI_Line5);
                EXTI->IMR &= ~(EXTI_Line6);
                EXTI->IMR &= ~(EXTI_Line7);
                EXTI->IMR &= ~(EXTI_Line11);
                irq_flag = IRQ_OFF;
            }
            //NVIC_DisableIRQ(EXTI9_5_IRQn);
            //NVIC_DisableIRQ(EXTI15_10_IRQn);
            ir_receive_data();
            irUsing |= IRUSING_CAM2_VISCA;    
        }
        EXTI_ClearITPendingBit(EXTI_Line4); // 清除中断标志
    }
    //EXTI->IMR |= EXTI_Line4;	// 使能外部中断
    //EXTI->IMR |= EXTI_Line5;	// 使能外部中断
    //EXTI->IMR |= EXTI_Line6;	// 使能外部中断
    //EXTI->IMR |= EXTI_Line7;	// 使能外部中断
    //EXTI->IMR |= EXTI_Line11;	// 使能外部中断
    
}

int press_trigger = 1;
int press_flag = 0;
int short_press_flag = 0;
unsigned int press_trigger_time = 0;
extern unsigned int s_numOf100us;
void EXTI9_5_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
        if (!(irUsing & IRUSING_FLAG) || (irUsing & IRUSING_CAM1_HDBT))
		{
            if (IRQ_ON == irq_flag)
            {

                EXTI_ClearITPendingBit(EXTI_Line4); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line6); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line7); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line11); // 清除中断标志
                EXTI->IMR &= ~(EXTI_Line4);
                EXTI->IMR &= ~(EXTI_Line6);
                EXTI->IMR &= ~(EXTI_Line7);
                EXTI->IMR &= ~(EXTI_Line11);
                irq_flag = IRQ_OFF;
            }

			//NVIC_DisableIRQ(EXTI4_IRQn);
			//NVIC_DisableIRQ(EXTI15_10_IRQn);			
			ir_receive_data();
			irUsing |= IRUSING_CAM1_HDBT;
		}
        EXTI_ClearITPendingBit(EXTI_Line5); // 清除中断标志

        //EXTI->IMR |= EXTI_Line4;	// 使能外部中断
        //EXTI->IMR |= EXTI_Line5;	// 使能外部中断
        //EXTI->IMR |= EXTI_Line6;	// 使能外部中断
        //EXTI->IMR |= EXTI_Line7;	// 使能外部中断
        //EXTI->IMR |= EXTI_Line11;	// 使能外部中断
	}

    /* PA6 CAMERA1_VISCA_IR */
	if (EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
		if (!(irUsing & IRUSING_FLAG) || (irUsing & IRUSING_CAM1_VISCA))
		{
            if (IRQ_ON == irq_flag)
            {
                EXTI_ClearITPendingBit(EXTI_Line4); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line5); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line7); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line11); // 清除中断标志
                EXTI->IMR &= ~(EXTI_Line5);
                EXTI->IMR &= ~(EXTI_Line4);
                EXTI->IMR &= ~(EXTI_Line7);
                EXTI->IMR &= ~(EXTI_Line11);
                irq_flag = IRQ_OFF;
            }
			//NVIC_DisableIRQ(EXTI4_IRQn);
			//NVIC_DisableIRQ(EXTI15_10_IRQn);
			ir_receive_data();
			irUsing |= IRUSING_CAM1_VISCA;
		}
        EXTI_ClearITPendingBit(EXTI_Line6); // 清除中断标志
        //EXTI->IMR |= EXTI_Line4;	// 使能外部中断
        //EXTI->IMR |= EXTI_Line5;	// 使能外部中断
        //EXTI->IMR |= EXTI_Line6;	// 使能外部中断
        //EXTI->IMR |= EXTI_Line7;	// 使能外部中断
        //EXTI->IMR |= EXTI_Line11;	// 使能外部中断

	}    

    #if 0
	/* 单板检测am5728上下电，中断 */
	if (EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
        if (press_trigger) // 下降沿处理      
        {
            press_trigger_time = s_numOf100us;
            press_trigger = 0;
            short_press_flag = 1;
            press_flag = 1;   

        }
        EXTI_ClearITPendingBit(EXTI_Line7); // 清除中断标志	
	}	
    #endif

    /* CAMERA2_HDBT_IR PA7 */
    if (EXTI_GetITStatus(EXTI_Line7) != RESET)
    {
 		if (!(irUsing & IRUSING_FLAG) || (irUsing & IRUSING_CAM2_HDBT))
		{
            if (IRQ_ON == irq_flag)
            {
                EXTI_ClearITPendingBit(EXTI_Line4); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line5); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line6); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line11); // 清除中断标志
                EXTI->IMR &= ~(EXTI_Line5);
                EXTI->IMR &= ~(EXTI_Line6);
                EXTI->IMR &= ~(EXTI_Line4);
                EXTI->IMR &= ~(EXTI_Line11);
                irq_flag = IRQ_OFF;
            }
			//NVIC_DisableIRQ(EXTI4_IRQn);
			//NVIC_DisableIRQ(EXTI15_10_IRQn);			
			ir_receive_data();
			irUsing |= IRUSING_CAM2_HDBT;
		}
        EXTI_ClearITPendingBit(EXTI_Line7); // 清除中断标志
        //EXTI->IMR |= EXTI_Line4;	// 使能外部中断
        //EXTI->IMR |= EXTI_Line5;	// 使能外部中断
        //EXTI->IMR |= EXTI_Line6;	// 使能外部中断
        //EXTI->IMR |= EXTI_Line7;	// 使能外部中断
        //EXTI->IMR |= EXTI_Line11;	// 使能外部中断
    }
}

void EXTI15_10_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line11) != RESET)
	{
		if (!(irUsing & IRUSING_FLAG) || (irUsing & IRUSING_FRP))
		{
            if (IRQ_ON == irq_flag)
            {
                EXTI_ClearITPendingBit(EXTI_Line4); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line5); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line6); // 清除中断标志
                EXTI_ClearITPendingBit(EXTI_Line7); // 清除中断标志
                EXTI->IMR &= ~(EXTI_Line5);
                EXTI->IMR &= ~(EXTI_Line6);
                EXTI->IMR &= ~(EXTI_Line7);
                EXTI->IMR &= ~(EXTI_Line4);
                irq_flag = IRQ_OFF;
            }
			//EXTI->IMR &= ~(EXTI_Line4); // 屏蔽外部中断
			//NVIC_DisableIRQ(EXTI4_IRQn);
			//NVIC_DisableIRQ(EXTI9_5_IRQn);		
			ir_receive_data();
			irUsing |= IRUSING_FRP; 			
		}
        EXTI_ClearITPendingBit(EXTI_Line11); // 清除中断标志
		
	}
}

/* 设置0.1ms 中断 */
void TIM2_IRQHandler(void)
{
	ucTime2Flag++;
    ucTime1Flag++;
    irq_timecnt++;
	
    if (3000 == irq_timecnt)
    {
        if((camera_ir_flag & 0x20)  != 0x20)//没有屏蔽标志才使能
        {
            if (0 == (EXTI->IMR &  EXTI_Line4))EXTI->IMR |= EXTI_Line4;	// 使能外部中断
        }
        
        if (0 == (EXTI->IMR &  EXTI_Line5))EXTI->IMR |= EXTI_Line5;	// 使能外部中断
        
        if((camera_ir_flag & 0x10)  != 0x10)//没有屏蔽标志才使能
        {
            if (0 == (EXTI->IMR &  EXTI_Line6))EXTI->IMR |= EXTI_Line6;	// 使能外部中断
        }

        if (0 == (EXTI->IMR &  EXTI_Line7))EXTI->IMR |= EXTI_Line7;	// 使能外部中断
        if (0 == (EXTI->IMR &  EXTI_Line8))EXTI->IMR |= EXTI_Line11;	// 使能外部中断
        irq_timecnt = 0;
        irq_flag = IRQ_ON;
    }
	if ((irUsing & IRUSING_FLAG) && (IR_PERIOD == ucTime2Flag)) // 防止自动中断，卡死
	{
		irUsing &= ~IRUSING_FLAG;
		//NVIC_EnableIRQ(EXTI4_IRQn);
		//NVIC_EnableIRQ(EXTI9_5_IRQn);
		//NVIC_EnableIRQ(EXTI15_10_IRQn);			
		ucTime2Flag = 0;
		ucTime1Flag = 0;
	}
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

/**
 * 每次电平转变触发 
 *
 * 红外数据解码
 * 一个脉冲对应560us的连续载波
 * 引导码：
 *		9ms低 + 4.5ms高 
 * 传输：
 *		逻辑1传输需要2.25ms(560us 脉冲+1680us 低电平)
 *		逻辑 0 的传输需要 1.125ms（560us 脉冲+560us 低电平）
 * 接收端：
 *		收到脉冲的时候为低电平，在没有脉冲的时候为高电平
 *		逻辑1：560us 低+1680us 高
 *		逻辑0：560us 低+560us 高
 * 重复码：
 *		9ms低 + 2.25ms高 + 0.56ms低 + 97.94ms高
 * receive_code接收到数据分别是地址、地址反码、数据、数据反码
 * TODO 一直按，这个没有处理
 */

static void ir_receive_data(void)
{
	// 上升沿捕获
	if ((GPIOD11_DATA && (EXTI_GetITStatus(EXTI_Line11) != RESET)) 
		|| (GPIOA4_DATA && (EXTI_GetITStatus(EXTI_Line4) != RESET)) 
		|| (GPIOA5_DATA && (EXTI_GetITStatus(EXTI_Line5) != RESET))			
		|| (GPIOA6_DATA && (EXTI_GetITStatus(EXTI_Line6) != RESET))			
		|| (GPIOA7_DATA && (EXTI_GetITStatus(EXTI_Line7) != RESET)))			
	{	
		GPIO_SetBits(GPIOC,GPIO_Pin_3); // 红外发送
		GPIO_SetBits(GPIOC,GPIO_Pin_2);
//        GPIO_SetBits(GPIOD, GPIO_Pin_3); // CAM1_IR_OUT
//		GPIO_SetBits(GPIOD,GPIO_Pin_4); // CAM2_IR_OUT       
        
		ucTime2Flag = 0;		// 清空计数
		irSta |= RISING_FLAG;	// 标记上升沿被捕获

        if(ucTime1Flag >= IR_NOISE_MIN && ucTime1Flag <= IR_NOISE_MAX)
        {
            sysflag_S = 1;
        }

		ucTime1Flag = 0;		// 清空计数


    }
	else 						// 下降沿捕获
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_3);	// 红外发送
		GPIO_ResetBits(GPIOC, GPIO_Pin_2);
//		GPIO_ResetBits(GPIOD, GPIO_Pin_3);	// CAM1_IR_OUT
//		GPIO_ResetBits(GPIOD, GPIO_Pin_4);  // CAM2_IR_OUT  

		if (irSta & RISING_FLAG)
		{
			if (irSta & SYS_CODE_FLAG)			// 接收到了引导码
			{
				if (ucTime2Flag >= LOGIC_0_HIGH_MIN && ucTime2Flag <= LOGIC_0_HIGH_MAX) // 逻辑0
				{
					rmRec <<= 1;
					//rmRec |= 0;
				}
				else if (ucTime2Flag >= LOGIC_1_HIGN_MIN && ucTime2Flag <= LOGIC_1_HIGN_MAX) // 逻辑1
				{
					rmRec <<= 1;
					rmRec |= 1;
				}
			}
            
			else if ( ucTime2Flag >= SYS_CODE_HIGH_MIN && ucTime2Flag <= SYS_CODE_HIGH_MAX) // 同步码
			{
			    if(sysflag_S)
                {         
    				irSta |= SYS_CODE_FLAG;
    				rmRec = 0;
    				keyCnt = 0;
    				idx = 0;				
                }
            }
            else if (ucTime2Flag >= REPEAT_CODE_HIGH_MIN && ucTime2Flag <= REPEAT_CODE_HIGH_MAX) // 重复码
            {
                keyCnt++;
                irSta &= (~SYS_CODE_FLAG);
            }
            else if(ucTime2Flag < IR_NOISE || sysflag_S) //干扰信号
            {  
                // camera1 中断
                if(irUsing & IRUSING_CAM1_VISCA)
                { 
                    camera_ir_flag |= 0x10;
                    if(EXTI->IMR & EXTI_Line6)
                    {
                        //printf("close cam1 %d T2=%d sysflag_s=%d\n", s_numof1s, ucTime2Flag, sysflag_S);
                        camera1_time = s_numof1s;
                        IR_NOISE_DETECT = 6;
                        //屏蔽camrea1 中断
                        EXTI->IMR &= ~(EXTI_Line6);
                    }
            
                //camera_ir_flag &= ~0x01;
                }

                if(irUsing & IRUSING_CAM2_VISCA)
                {
                    camera_ir_flag |= 0x20;
                    if(EXTI->IMR & EXTI_Line4)
                    {
                        //printf("close cam2 %d T2=%d sysflag_S=%d\n", s_numof1s, ucTime2Flag, sysflag_S);
                        camera2_time = s_numof1s;
                        IR_NOISE_DETECT = 6;
                        //屏蔽camrea2 中断
                        EXTI->IMR &= ~(EXTI_Line4);
                    }

                //camera_ir_flag &= ~0x02; 
                }
            
            }

			if (0 == keyCnt)
			{
				idx++;
			}
			
			if (GET_DATA_NUM == idx)
			{
                irq_flag = IRQ_ON;
                if((camera_ir_flag & 0x20)  != 0x20)//没有屏蔽标志才使能
                {
                    if(0 == (EXTI->IMR &  EXTI_Line4))EXTI->IMR |= EXTI_Line4;	// 使能外部中断
                }
                if(0 == (EXTI->IMR &  EXTI_Line5))EXTI->IMR |= EXTI_Line5;	// 使能外部中断
                if((camera_ir_flag & 0x10)  != 0x10)//没有屏蔽标志才使能
                {
                    if(0 == (EXTI->IMR &  EXTI_Line6))EXTI->IMR |= EXTI_Line6;	// 使能外部中断
                }
                if(0 == (EXTI->IMR &  EXTI_Line7))EXTI->IMR |= EXTI_Line7;	// 使能外部中断
                if(0 == (EXTI->IMR &  EXTI_Line8))EXTI->IMR |= EXTI_Line11;	// 使能外部中断
				if (irSta & SYS_CODE_FLAG)
				{
					irSta |= RECEIVE_OK;
					irUsing &= ~IRUSING_FLAG;
				}
			}
            
        }
		ucTime2Flag = 0;
        ucTime1Flag = 0;
        sysflag_S = 0;
		
		irSta &= ~RISING_FLAG;
	}

}

/* 每次电平转变触发，数据解析 */
/**
 * 红外数据解码
 * 一个脉冲对应560us的连续载波
 * 引导码：
 *		9ms低 + 4.5ms高 
 * 传输：
 *		逻辑1传输需要2.25ms(560us 脉冲+1680us 低电平)
 *		逻辑 0 的传输需要 1.125ms（560us 脉冲+560us 低电平）
 * 接收端：
 *		收到脉冲的时候为低电平，在没有脉冲的时候为高电平
 *		逻辑1：560us 低+1680us 高
 *		逻辑0：560us 低+560us 高
 * 重复码：
 *		9ms低 + 2.25ms高 + 0.56ms低 + 97.94ms高
 * receive_code接收到数据分别是地址、地址反码、数据、数据反码
 * TODO 一直按，这个没有处理
 */
void ir_decode_data(void)
{
	u8 t1 = 0;
	u8 t2 = 0;
	int gpio_value = 0;

	if (irSta & RECEIVE_OK) 
	{
		t1 = rmRec >> 24;
		t2 = (rmRec>>16) & 0xff;
		if (t1 == (u8)~t2)		// 校验地址码
		{
			t1 = rmRec >> 8;
			t2 = rmRec;
			if (t1 == (u8)~t2)	// 控制码校验
			{
				reg_val[SYS_IR_VAL] = t1;
				if (0xd0 == reg_val[SYS_IR_VAL])    // 关机键值
				{
                    gpio_value = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6);   
                    if (gpio_value == 0)
                    {
                        power_on_system();
                    }
                }    

                led_blink_ir();
				printf("val:0x%x \n\r", reg_val[SYS_IR_VAL]);

			}
		}
	
		irSta &= ~(RECEIVE_OK | SYS_CODE_FLAG);
		/*
		if ((0 == *key_value) || ((irSta&0x80) == 0))
		{
			irSta &= ~(1<<6);
			keyCnt = 0;
		}
		*/
	}
	
	return;
}
  
void ir_check_get_data(void)
{
	if (reg_val[SYS_IR_CTL] & BIT0)
	{
		ir_decode_data();
		reg_val[SYS_IR_CTL] &= ~BIT0;
	}
}

void ir_init(void)
{
	ir_exti_init();
	ir_tim2_init(PRESCALER_VALUE, PERIOD_VALUE);
}

