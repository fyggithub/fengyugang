#include "led.h"
#include "debug_usart.h"
#include "myiic.h"
#include "timer.h"

#define RED_LED_OFF GPIO_SetBits(GPIOD, GPIO_Pin_13)       
#define RED_LED_ON GPIO_ResetBits(GPIOD, GPIO_Pin_13) 
#define GREEN_LED_OFF GPIO_SetBits(GPIOD, GPIO_Pin_14)       
#define GREEN_LED_ON GPIO_ResetBits(GPIOD, GPIO_Pin_14)  
#define BLUE_LED_OFF GPIO_SetBits(GPIOD, GPIO_Pin_15)       
#define BLUE_LED_ON GPIO_ResetBits(GPIOD, GPIO_Pin_15)  

#define GPIOD13_DATA			GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_13)	// RED
#define GPIOD14_DATA			GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_14)	// GREEN
#define GPIOD15_DATA			GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_15)	// BLUE
#define COUNT_IR       2
#define LED_ALL_OFF     4

extern unsigned char reg_val[I2C_REG_NUM];
extern int led_enable_r; // breathing
extern int led_enable_g;
extern int led_enable_b;
int led_loop_blink_r;
int led_loop_blink_g;
int led_loop_blink_b;
int led_loop_count_r = 0;
int led_loop_count_g = 0;
int led_loop_count_b = 0;
int led_blink_ir_enable = 0;
int led_blink_color = 0;
int led_blink_count = 0;

static void led_disable_all_flag(void)
{
    led_loop_blink_r = 0;
    led_loop_blink_g = 0;
    led_loop_blink_b = 0;
    
    led_enable_r = 0; // breathing
    led_enable_g = 0;
    led_enable_b = 0;
}

void led_ctl(int index, int on) 
{
	if (on)
	{
		switch(index)
		{
			case RED_LED:	
				TIM4->CCR2 = LED_ON_T;
                TIM4->CCR3 = LED_OFF_T;
                TIM4->CCR4 = LED_OFF_T;
				break;

			case GREEN_LED:	
				TIM4->CCR2 = LED_OFF_T;
                TIM4->CCR3 = LED_ON_T;
                TIM4->CCR4 = LED_OFF_T;

				break;

			case BLUE_LED:
				TIM4->CCR2 = LED_OFF_T;
                TIM4->CCR3 = LED_OFF_T;
                TIM4->CCR4 = LED_ON_T;
				break;
		}
	}
	else 
	{
	    led_disable_all_flag();
        TIM4->CCR2 = LED_OFF_T;
        TIM4->CCR3 = LED_OFF_T;
        TIM4->CCR4 = LED_OFF_T;	
	}
} 

void led_blink_ir_timer(void)
{
    if (!led_blink_ir_enable)
        return;
    if (COUNT_IR != led_blink_count++)
        return;
    
    led_blink_count = 0;
    led_blink_ir_enable = 0;     
    switch(led_blink_color)    
    {
        case RED_LED:      
            TIM4->CCR2 = LED_ON_T;
            break;

        case GREEN_LED:
            TIM4->CCR3 = LED_ON_T;  
            break;

        case BLUE_LED: 
            TIM4->CCR4 = LED_ON_T;
            break;     

        case LED_ALL_OFF:  // 当灯全部灭时，默认闪烁蓝色
            TIM4->CCR4 = LED_OFF_T;                
            break;   

        default:
            break;
    }
}

/* 按下红外，灯闪烁 */
void led_blink_ir(void)
{
    led_blink_ir_enable = 1;
    
    if (!GPIOD13_DATA)
    {
        led_blink_color = RED_LED;
        TIM4->CCR2 = LED_OFF_T;
        TIM4->CCR3 = LED_OFF_T;
        TIM4->CCR4 = LED_OFF_T;         
    }
    else if (!GPIOD14_DATA)
    {
        led_blink_color = GREEN_LED;
        TIM4->CCR2 = LED_OFF_T;
        TIM4->CCR3 = LED_OFF_T;
        TIM4->CCR4 = LED_OFF_T;               
    }
    else if (!GPIOD15_DATA)
    {
        led_blink_color = BLUE_LED;
        TIM4->CCR2 = LED_OFF_T;
        TIM4->CCR3 = LED_OFF_T;
        TIM4->CCR4 = LED_OFF_T;               
    }    
    else       
    {
        TIM4->CCR4 = LED_ON_T; 
        led_blink_color = LED_ALL_OFF;     
    }     
}

static void led_on_off(int led_type, int led_reg)
{
	if (reg_val[led_reg] & BIT7) 
	{
		led_ctl(led_type, reg_val[led_reg] & BIT0);	
		reg_val[led_reg] &= ~BIT7;
	}
}

static void led_breathing(int led_reg, int *led_breath_flag)
{
	if (reg_val[led_reg] & BIT6) 
	{
		if (reg_val[led_reg] & BIT1)
        {
            led_disable_all_flag();
            TIM4->CCR2 = LED_OFF_T;
            TIM4->CCR3 = LED_OFF_T;
            TIM4->CCR4 = LED_OFF_T;
            *led_breath_flag = 1;
        }      
        else
        {
            led_enable_r = 0;
            led_enable_g = 0;
            led_enable_b = 0;
            *led_breath_flag = 0;
            TIM4->CCR2 = LED_OFF_T;
            TIM4->CCR3 = LED_OFF_T;
            TIM4->CCR4 = LED_OFF_T;	                        
        }
		reg_val[led_reg] &= ~BIT6;
	}
}

static void led_loop_blink(int led_reg, int *led_loop_flag)
{
	if (reg_val[led_reg] & BIT5) 
	{
		if (reg_val[led_reg] & BIT2)
        {
            led_disable_all_flag();
            TIM4->CCR2 = LED_OFF_T;
            TIM4->CCR3 = LED_OFF_T;
            TIM4->CCR4 = LED_OFF_T;
            *led_loop_flag = 1;
        }      
        else
        {
            *led_loop_flag = 0;
            led_loop_blink_r = 0;
            led_loop_blink_g = 0;
            led_loop_blink_b = 0;                    
            TIM4->CCR2 = LED_OFF_T;
            TIM4->CCR3 = LED_OFF_T;
            TIM4->CCR4 = LED_OFF_T;	                        
        }
		reg_val[led_reg] &= ~BIT5;
	}    
}

void led_loop_blink_timer(void)
{
    if (led_loop_blink_r)       
    {
        TIM4->CCR2 = LED_OFF_T;
        if (led_loop_count_r++ >= reg_val[LED_COUNT_R])
        {
            TIM4->CCR2 = LED_ON_T;
            led_loop_count_r = 0;
        }
    }

    if (led_loop_blink_g)       
    {
        TIM4->CCR3 = LED_OFF_T;
        if (led_loop_count_g++ >= reg_val[LED_COUNT_G])
        {
            TIM4->CCR3 = LED_ON_T;
            led_loop_count_g = 0;
        }
    }

    if (led_loop_blink_b)       
    {
        TIM4->CCR4 = LED_OFF_T;
        if (led_loop_count_b++ >= reg_val[LED_COUNT_B])
        {
            TIM4->CCR4 = LED_ON_T;
            led_loop_count_b = 0;
        }
    }    
}

/*================ test code=======================*/
#if 1
int ledpwmval_test = 500;
int dir_test = 0;
static void led_bright_test(void)
{
    if (0 == reg_val[LED_TEST1])     // 0x72
        return ;  

    if (1 == reg_val[LED_TYPE]) // 0x71
        TIM4->CCR2 = (reg_val[LED_BRIGHT] * 4); // 0x73

    if (2 == reg_val[LED_TYPE])
        TIM4->CCR3 = (reg_val[LED_BRIGHT] * 4);

    if (3 == reg_val[LED_TYPE])
        TIM4->CCR4 = (reg_val[LED_BRIGHT] * 4);    
}

void led_breathing_test(void)
{
    if (0 == reg_val[LED_TEST])    // 0x6e
        return ;

    if (1 == reg_val[LED_TYPE])   // 0x71
    {
        TIM4->CCR2 = ledpwmval_test;    //根据PWM表修改定时器的比较寄存器值
        
        if (dir_test)
        {  
            if (ledpwmval_test <= reg_val[LED_INTERVAL]) // 0x74
                ledpwmval_test += reg_val[LED_ADD_1]; // 0x6f
            else
                ledpwmval_test += reg_val[LED_ADD_2];       // 0x70
        }
        else 
        {
            if (ledpwmval_test <= reg_val[LED_INTERVAL])
                ledpwmval_test -= reg_val[LED_ADD_1];
            else
                ledpwmval_test -= reg_val[LED_ADD_2];            
        }

        if (ledpwmval_test >= 500)
            dir_test = 0;
        if (0 >= ledpwmval_test)
        {
            dir_test = 1;  
            ledpwmval_test = 0;
        }
    }

    if (2 == reg_val[LED_TYPE])
    {
        TIM4->CCR3 = ledpwmval_test;    //根据PWM表修改定时器的比较寄存器值
        
        if (dir_test)
        {  
            if (ledpwmval_test <= reg_val[LED_INTERVAL]) // 0x74
                ledpwmval_test += reg_val[LED_ADD_1]; // 0x6f
            else
                ledpwmval_test += reg_val[LED_ADD_2];       // 0x70
        }
        else 
        {
            if (ledpwmval_test <= reg_val[LED_INTERVAL])
                ledpwmval_test -= reg_val[LED_ADD_1];
            else
                ledpwmval_test -= reg_val[LED_ADD_2];            
        }

        if (ledpwmval_test >= 500)
            dir_test = 0;
        if (0 >= ledpwmval_test)
        {
            dir_test = 1;  
            ledpwmval_test = 0;
        }

    }

    if (3 == reg_val[LED_TYPE])
    {
        TIM4->CCR4 = ledpwmval_test;    //根据PWM表修改定时器的比较寄存器值
        
         if (dir_test)
        {  
            if (ledpwmval_test <= reg_val[LED_INTERVAL]) // 0x74
                ledpwmval_test += reg_val[LED_ADD_1]; // 0x6f
            else
                ledpwmval_test += reg_val[LED_ADD_2];       // 0x70
        }
        else 
        {
            if (ledpwmval_test <= reg_val[LED_INTERVAL])
                ledpwmval_test -= reg_val[LED_ADD_1];
            else
                ledpwmval_test -= reg_val[LED_ADD_2];            
        }

        if (ledpwmval_test >= 500)
        {
            dir_test = 0;
            ledpwmval_test = 500;
        }
        if (0 >= ledpwmval_test)
        {
            dir_test = 1;  
            ledpwmval_test = 0;
        }
    }      
}
#endif
/*================ test code=======================*/

void update_led(void)
{
    led_on_off(RED_LED, RED_LED_CTL);
    led_on_off(GREEN_LED, GREEN_LED_CTL);
    led_on_off(BLUE_LED, BLUE_LED_CTL);

    led_breathing(RED_LED_CTL, &led_enable_r);
    led_breathing(GREEN_LED_CTL, &led_enable_g);
    led_breathing(BLUE_LED_CTL, &led_enable_b); 

    led_loop_blink(RED_LED_CTL, &led_loop_blink_r);
    led_loop_blink(GREEN_LED_CTL, &led_loop_blink_g);
    led_loop_blink(BLUE_LED_CTL, &led_loop_blink_b);
    led_bright_test();  // for test
}

