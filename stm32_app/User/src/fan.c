#include "timer.h"
#include "gpio.h"
#include "debug_usart.h"
#include "fan.h"
#include "myiic.h"

//#define FAN_Level_0   60
//#define FAN_Level_1   75
//#define FAN_Level_2   90
//#define FAN_Level_3   100

#define FAN_Level_0   40
#define FAN_Level_1   25
#define FAN_Level_2   10
#define FAN_Level_3   0

extern unsigned char reg_val[I2C_REG_NUM]; 
static unsigned int fan_start_time = 0;
extern unsigned int s_numOf100us;
static int fan_auto_ctrl = 1;
/**
 * arr: 自动重装值
 * psc: 时钟预分频数
 * 输入时钟  72M
 * Tout = ((arr+1)*(psc+1))/Tclk  (us)
 */
void fan_init(u16 arr, u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef       TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);						//使能定时器1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE); //使能GPIO和AFIO复用功能时钟 
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE);						//重映射 TIM3_CH2->PE11

	//设置该引脚为复用输出功能,输出 TIM1 CH2 的 PWM 脉冲波形 GPIOE.11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 					//TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 			//复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure); 						//初始化 GPIO

    //初始化TIM1
	TIM_TimeBaseStructure.TIM_Period = arr; 					//设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler = psc;					//设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//TIM向上计数模式
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    /* TIM1_OC1模块设置(设置1通道占空比)  */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OCInitStructure.TIM_Pulse = 60;	
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	
    TIM_OC2PreloadConfig(TIM1,TIM_OCPreload_Enable); 
    //TIM_ARRPreloadConfig(TIM1, ENABLE); 

    TIM_CtrlPWMOutputs(TIM1, ENABLE);	// TIM1_OC通道输出PWM(一定要加)
    TIM_Cmd(TIM1, ENABLE);		 		// TIM1开启 		
}

/* 设置占空比 */ 
void fan_setduty(uint16_t Compare2)
{
	TIM_SetCompare2(TIM1, Compare2);
}

void fan_set_speed(void)
{
    char temp;
    temp = reg_val[SYS_TEMP_H];

    if(GPIO_ReadOutputDataBit(GPIOD, GPIO_Pin_6) == 0)
    {
        reg_val[SYS_FAN_SPEED] = FAN_STOP;
        fan_setduty(FAN_STOP);  // close fan when system power down
        return ;
    }

//    if (0x80 & reg_val[SYS_CTL_FAN]) // 负数
    if (0x80 & reg_val[SYS_TEMP_H]) // 负数
	{
        reg_val[SYS_FAN_SPEED] = FAN_STOP;
//        fan_setduty(FAN_STOP);
		fan_setduty(FAN_Level_0);
	}
    else 
    {
        if(temp <= 40)
        {
            reg_val[SYS_FAN_SPEED] = FAN_Level_0;
            fan_setduty(FAN_Level_0);
        }
        else if(temp > 40 && temp <= 44)
        {
            reg_val[SYS_FAN_SPEED] = FAN_Level_1;
            fan_setduty(FAN_Level_1);
        }
        else if(temp > 44 && temp <= 48)     
        {
            reg_val[SYS_FAN_SPEED] = FAN_Level_2;
            fan_setduty(FAN_Level_2);
        }
        else
        {
            reg_val[SYS_FAN_SPEED] = FAN_Level_3;
            fan_setduty(FAN_Level_3);
        }
    }
}

void update_fan_speed(void)
{
	static int fan_flag = 1;
	
    if (1 == fan_flag)
    {
        fan_start_time = s_numOf100us;
        fan_flag = 0;       
    }   

	if (BIT7 & reg_val[SYS_FAN_AUTO_CTRL_DISABLE])
	{
		fan_auto_ctrl = 0;
	}
	else
	{
		fan_auto_ctrl = 1;
	}

    if (greater_times(fan_start_time, s_numOf100us, 20000)) // 2s
    {
        fan_flag = 1; 
		if(fan_auto_ctrl)    			//自动调节温度
		{
	        fan_set_speed();    	      
		}
    }	
	
    if (BIT7 & reg_val[SYS_CTL_FAN])
    {
        fan_setduty(reg_val[SYS_FAN_SPEED]);  
        reg_val[SYS_CTL_FAN] &= ~BIT7;
    }
}

