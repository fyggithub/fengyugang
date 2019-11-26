#include "timer.h"
#include "gpio.h"
#include "debug_usart.h"
#include "wdg.h"
#include "led.h"

extern u8 led_blink[3];
//unsigned int s_numOf40us = 0;
unsigned int s_numOf100us = 0;
unsigned int s_num = 0;
unsigned int s_numof1s = 0;
unsigned int tim4_count = 0;
unsigned int iwdg_count = 0;

/*
 * 通用定时器3中断初始化
 * 这里时钟选择为APB1的2倍，而APB1为36M
 * arr：自动重装值。
 * psc：时钟预分频数
 * 这里使用的是定时器3!
 * Tout= ((arr+1)*(psc+1))/Tclk 
 * 这里设置为0.1ms   prescaler: 100-1, period: 72-1
 */
void TIM3_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; 					//设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 					//设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 	//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 			//根据指定的参数初始化TIMx的时间基数单位
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); 					//使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  			//TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  	//先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; 			//从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 			//IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  							//初始化NVIC寄存器

	TIM_Cmd(TIM3, ENABLE);  //使能TIMx					 
}

/* 定时器3中断服务程序 */
void TIM3_IRQHandler(void)   //TIM3中断
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx更新中断标志 
        s_num++;
        s_numOf100us++;
        if (MAX_VALUE_TIME == s_numOf100us)
        {
			s_numOf100us = 0;
        }	

        if(s_num >= 10000)
        {
            s_numof1s++;
            s_num = 0;
        }
        if(MAX_VALUE_TIME / 2 == s_numof1s)
        {
            s_numof1s = 0;
        }
            
        if (greater_times(iwdg_count, s_numOf100us, 30000))
        {
            IWDG_Feed();
            iwdg_count = s_numOf100us;
        }
    }
}

/** TIM4 PWM部分初始化 
 * PWM输出初始化
 * arr：自动重装值
 * psc：时钟预分频数
 * Tout= ((arr+1)*(psc+1))/Tclk 
 */
void tim4_pwn_init(u16 arr, u16 psc)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE); 
    GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); //Timer4 重映射  TIM4_CH2->PD13  

    //设置该引脚为复用输出功能,输出 TIM4 CH2 的 PWM 脉冲波形 GPIOD.13
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 					//TIM_CH2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 			//复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure); 						//初始化 GPIO    

    //初始化TIM4
    TIM_TimeBaseStructure.TIM_Period = arr; 					//设置在下一个更新事件装入活动的自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler = psc;					//设置用来作为TIMx时钟频率除数的预分频值 
    //TIM_TimeBaseStructure.TIM_Period = 9; 					//设置在下一个更新事件装入活动的自动重装载寄存器周期的值
    //TIM_TimeBaseStructure.TIM_Prescaler = 71;					//设置用来作为TIMx时钟频率除数的预分频值 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//TIM向上计数模式
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);    

    /* TIM1_OC1模块设置(设置1通道占空比)  */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
    TIM_OCInitStructure.TIM_Pulse = 60;	
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);    

    TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable); 
    //TIM_ARRPreloadConfig(TIM4, ENABLE); 

    TIM_CtrlPWMOutputs(TIM4, ENABLE);	// TIM1_OC通道输出PWM(一定要加)
    TIM_Cmd(TIM4, ENABLE);	        
}

/* 配置TIM4复用输出PWM时用到的I/O */
static void TIM4_GPIO_Config(void) 
{
  GPIO_InitTypeDef GPIO_InitStructure;

 // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); 
  GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); //Timer4 重映射  TIM4_CH2->PD13 

  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		    // 复用推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}

/* 配置嵌套向量中断控制器NVIC */
static void NVIC_Config_PWM(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Configure one bit for preemption priority */
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  /* 配置TIM3_IRQ中断为中断源 */
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

static void TIM4_Mode_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;																				

    /* 设置TIM4CLK 时钟为72MHZ */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);        //使能TIM4时钟

    /* 基本定时器配置 */		             
    TIM_TimeBaseStructure.TIM_Period = ARR_TIM4;       		    //当定时器从0计数到ARR_TIM4，即为ARR_TIM4次，为一个定时周期
    TIM_TimeBaseStructure.TIM_Prescaler = PSC_TIM4;	    	    //设置预分频：
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;	//设置时钟分频系数：不分频(这里用不到)
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* PWM模式配置 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	    		//配置为PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//使能输出
    TIM_OCInitStructure.TIM_Pulse = 0;								//设置初始PWM脉冲宽度为0	
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;  	    //当定时器计数值小于CCR1_Val时为高电平

    TIM_OC2Init(TIM4, &TIM_OCInitStructure);	 					//使能通道2
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);				//使能预装载	

    TIM_OC3Init(TIM4, &TIM_OCInitStructure);	 					//使能通道3
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);				//使能预装载	

    TIM_OC4Init(TIM4, &TIM_OCInitStructure);	 					//使能通道4
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);				//使能预装载	
    
    TIM_ARRPreloadConfig(TIM4, ENABLE);			 					//使能TIM4重载寄存器ARR    
    //TIM_CtrlPWMOutputs(TIM4,ENABLE);                              //设置TIM4 的PWM 输出为使能 

    /* TIM4 enable counter */
    TIM_Cmd(TIM4, ENABLE);                   						//使能定时器4
    TIM_ITConfig(TIM4,TIM_IT_Update, ENABLE);						//使能update中断	
    NVIC_Config_PWM();												//配置中断优先级			
    led_ctl(BLUE_LED, LED_ON);                                      // 上电，灯默认蓝色

    //TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Disable);				//disable预装载	
    //TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Disable);				//disable预装载	
    //TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Disable);				//disable预装载	
}

/* TIM4 呼吸灯初始化 */
void TIM4_Breathing_Init(void)
{
	TIM4_GPIO_Config();
	TIM4_Mode_Config();	
}

int ledpwmval = 500;
u8 dir = 0;
int led_enable_r = 0;
int led_enable_b = 0;
int led_enable_g = 0;

/* 22ms */
void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {       
        if (led_enable_r)
        {
            TIM4->CCR2 = ledpwmval;    //根据PWM表修改定时器的比较寄存器值
            
            if (dir)
            {                    
                ledpwmval += 2;
            }
            else 
            {
                ledpwmval -= 2;
            }

            if (ledpwmval > 500)
                dir = 0;
            if (0 == ledpwmval)
                dir = 1;     
        }

        if (led_enable_g)
        {
            TIM4->CCR3 = ledpwmval;    //根据PWM表修改定时器的比较寄存器值
            
            if (dir)
            {                    
                ledpwmval += 2;
            }
            else 
            {
                ledpwmval -= 2;
            }

            if (ledpwmval > 500)
                dir = 0;
            if (0 == ledpwmval)
                dir = 1;     
        }

        if (led_enable_b)
        {
            TIM4->CCR4 = ledpwmval;    //根据PWM表修改定时器的比较寄存器值
            
            if (dir)
            {                    
                ledpwmval += 2;
            }
            else 
            {
                ledpwmval -= 2;
            }

            if (ledpwmval > 500)
                dir = 0;
            if (0 == ledpwmval)
                dir = 1;     
        }            

        led_blink_ir_timer();    
        led_loop_blink_timer();
        led_breathing_test();    // for test
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);    //必须要清除中断标志位
    }
}

