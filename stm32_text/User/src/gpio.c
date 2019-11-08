#include "sys.h"
#include "stm32f10x_gpio.h"
#include "gpio.h"
#include "delay.h"
#include "debug_usart.h"
#include "tmds181.h"
#include "myiic.h"
#include "usart.h"
#include "led.h"
#include "oled.h"
#include "fan.h"

typedef enum{
	PCIE_USB1_RESET = 0,
	PCIE_USB2_RESET,
	WIFI_RST,
	EXPRESS_RESET,
	EXPRESS_PWON,
	LAN_SWITCH_RESET,
	MAIN_POWER_CTL,
	RED_LED_OPT,
	GREEN_LED_OPT,
	BLUE_LED_OPT,
	MAX_GPIO,
}Read_Gpio_Level;

/* begin 5728 上下电控制 */
extern unsigned int s_numOf100us;
extern unsigned char reg_val[I2C_REG_NUM];
/* end 5728 上下电控制 */

#if 0
u8 led_blink[3] = {0x0};
void Gpio_setLed(LED_ID id, LED_STATE state)
{
    if(LED_ID_ACT == id)
    {
        switch (state)
        {
            case LED_STATE_OFF:  
                LED_ACT = 0x1;
                led_blink[0] = 0x0;
                break;

            case LED_STATE_ON:
                LED_ACT = 0x0;
                led_blink[0] = 0x0;
                break;

            case LED_STATE_BLINK:
                LED_ACT = 0x1;
                led_blink[0] = 0x1;
                break;
            default:
                DebugPrint("the input param is fail\n");
        }
    }
    else if(LED_ID_RUN == id)
    {
        switch (state)
        {
            case LED_STATE_OFF:  
                LED_RUN = 0x1;
                led_blink[1] = 0x0;
                break;

            case LED_STATE_ON:
                LED_RUN = 0x0;
                led_blink[1] = 0x0;
                break;

            case LED_STATE_BLINK:
                LED_RUN = 0x1;
                led_blink[1] = 0x1;
                break;
            default:
                DebugPrint("the input param is fail\n");
        }
    }
    else if(LED_ID_ERR == id)
    {
        switch (state)
        {
            case LED_STATE_OFF:  
                LED_ERR = 0x1;
                led_blink[2] = 0x0;
                break;

            case LED_STATE_ON:
                LED_ERR = 0x0;
                led_blink[2] = 0x0;
                break;

            case LED_STATE_BLINK:
                LED_ERR = 0x1;
                led_blink[2] = 0x1;
                break;
            default:
                DebugPrint("the input param is fail\n");
        }
    }
    else
    {
        DebugPrint("id is fail\n");
    }
}
#endif

/** 单板上电gpio初始化，一开始在boot给单板上电，
 * 
 * PD6		MAIN_12V_EN
 * PD7		PWR_KEY_RIGHT
 * 当电源按键按下是，PWR_KEY_RIGHT会检测到低电平，然后给MAIN_12V_EN这个脚拉低
 * PWR_KEY_RIGHT不按下时是高电平，按下是低电平
 * 正常上电时，MAIN_12V_EN是低
 */
void cpu_power_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

		/* am5728 复位检测脚 */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	//GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;		// 置成上拉输入
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;		// 浮空输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;     // 推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);  

    GPIO_SetBits(GPIOA, GPIO_Pin_11);
    GPIO_SetBits(GPIOA, GPIO_Pin_12);
	
	GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;     // 推挽输出
    GPIO_Init(GPIOE, &GPIO_InitStructure);  
	GPIO_SetBits(GPIOE, GPIO_Pin_7);
	
	/* pin_6: am5728 reset, others is PMIC reset pin */
    GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_6 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;		// 推挽输出
	GPIO_Init(GPIOD, &GPIO_InitStructure);	
    
	GPIO_ResetBits(GPIOD, GPIO_Pin_1);
	GPIO_ResetBits(GPIOD, GPIO_Pin_2);
	GPIO_SetBits(GPIOD, GPIO_Pin_3);
	GPIO_SetBits(GPIOD, GPIO_Pin_6);   
		
/*		GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_0;
		GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
		//GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;		// 置成上拉输入
		GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;		// 浮空输入
		GPIO_Init(GPIOD, &GPIO_InitStructure);	*/
}

u8 Gpio_Read_Fun(Read_Gpio_Level res)
{
	int read_value;
	switch(res)
	{
		case PCIE_USB1_RESET:{
							read_value = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11);
							if(read_value)		
								printf("PCIE_USB1_RESET set high!\n");
							else
								printf("PCIE_USB1_RESET set low!\n");
						}break;
		case PCIE_USB2_RESET:{
							read_value = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12);
							if(read_value)		
								printf("PCIE_USB2_RESET set high!\n");
							else
								printf("PCIE_USB2_RESET set low!\n");
						}break;
		case WIFI_RST:{
							read_value = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7);
							if(read_value)		
								printf("WIFI_RST set high!\n");
							else
								printf("WIFI_RST set low!\n");
						}break;
		case EXPRESS_RESET:{
							read_value = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_1);
							if(read_value)		
								printf("EXPRESS_RESET set high!\n");
							else
								printf("EXPRESS_RESET set low!\n");
						}break;
		case EXPRESS_PWON:{
							read_value = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2);
							if(read_value)		
								printf("EXPRESS_PWON set high!\n");
							else
								printf("EXPRESS_PWON set low!\n");
						}break;
		case LAN_SWITCH_RESET:{
							read_value = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3);
							if(read_value)		
								printf("LAN_SWITCH_RESET set high!\n");
							else
								printf("LAN_SWITCH_RESET set low!\n");
						}break;
		case MAIN_POWER_CTL:{
							read_value = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6);
							if(read_value)		
								printf("MAIN_POWER_CTL set high!\n");
							else
								printf("MAIN_POWER_CTL set low!\n");
						}break;
		case RED_LED_OPT:{
							read_value = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_13);
							if(read_value)		
								printf("RED_LED_OPT set high!\n");
							else
								printf("RED_LED_OPT set low!\n");
						}break;
		case GREEN_LED_OPT:{
							read_value = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_14);
							if(read_value)		
								printf("GREEN_LED_OPT set high!\n");
							else
								printf("GREEN_LED_OPT set low!\n");
						}break;
		case BLUE_LED_OPT:{
							read_value = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_15);
							if(read_value)		
								printf("BLUE_LED_OPT set high!\n");
							else
								printf("BLUE_LED_OPT set low!\n");
						}break;
		default:break;
	}
	return read_value;
}

void Gpio_Text(void)
{		
	Gpio_Read_Fun(PCIE_USB1_RESET);
	Gpio_Read_Fun(PCIE_USB2_RESET);
	Gpio_Read_Fun(WIFI_RST);
	Gpio_Read_Fun(EXPRESS_RESET);
	Gpio_Read_Fun(EXPRESS_PWON);
	Gpio_Read_Fun(LAN_SWITCH_RESET);
	Gpio_Read_Fun(MAIN_POWER_CTL);
	Gpio_Read_Fun(RED_LED_OPT);
	Gpio_Read_Fun(GREEN_LED_OPT);
	Gpio_Read_Fun(BLUE_LED_OPT);
	printf("\n");
	delay_ms(1000);
}

void IT6662_Close();
void IT6662_Reset();

int reboot_flag = 0;
unsigned int clean_uart_buf_time = 0;
void power_on_system(void)
{
	printf("****** power_on_system ******\n\r");
	led_ctl(BLUE_LED, LED_ON);
	oled_clear_diplay();    
	oled_print_logo();

	GPIO_SetBits(GPIOD, GPIO_Pin_6);	 // 对整个系统上电
	GPIO_SetBits(GPIOE, GPIO_Pin_2); 	 // poe power on
	GPIO_SetBits(GPIOA, GPIO_Pin_11);  //PCIE_USB1复位上电
	GPIO_SetBits(GPIOA, GPIO_Pin_12);  //PCIE_USB2复位上电
	GPIO_SetBits(GPIOE, GPIO_Pin_7);   //WIFI复位上电
	GPIO_SetBits(GPIOD, GPIO_Pin_3);   //LAN SWITCH复位上电

	delay_ms(1000);
	reboot_flag = 1;  
	clean_uart_buf_time = s_numOf100us;
}

static void power_off_system(void)
{
	printf("****** power_off_system ******\n\r");
	fan_setduty(0);    										//先关风扇
	delay_ms(1000);
	GPIO_ResetBits(GPIOD, GPIO_Pin_6);    // 对整个系统下电

	led_ctl(BLUE_LED, LED_OFF);
	oled_clear_diplay(); 
	GPIO_ResetBits(GPIOE, GPIO_Pin_2); // poe power down
	GPIO_ResetBits(GPIOA, GPIO_Pin_11);  //PCIE_USB1复位掉电
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);  //PCIE_USB2复位掉电
	GPIO_ResetBits(GPIOE, GPIO_Pin_7);   //WIFI复位掉电
	GPIO_ResetBits(GPIOD, GPIO_Pin_3);   //LAN SWITCH复位上电
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
}

/** 
 * interval = 100*times us 
 * 判断时间间隔是否大于times
 * 是，返回1
 * 否，返回0 
 */
int greater_times(unsigned int begin_time, unsigned int end_time, unsigned int times)
{
	if (end_time >= begin_time)
	{
		if ((end_time - begin_time) > times)
		{
			return 1;
		}
	}
	else
	{
		if (((MAX_VALUE_TIME - begin_time) + end_time) > times)
		{
			return 1;
		}
	}

    return 0;
}

void clean_uart_buf(void)
{
    if (reboot_flag)
    {
        if (greater_times(clean_uart_buf_time, s_numOf100us, 1000))
        {
            uart4_clean_buf();
            reboot_flag = 0;
        }    
    }
}

#if 0  // 中断实现开关机
extern int press_flag;
extern int on_off;
extern int press_trigger;
extern int short_press_flag;
extern unsigned int press_trigger_time;
unsigned int start_release_time;
int start_release = 1;;

static void power_on_off(void)
{
    int gpio_value = 1;

    /* 高电平，表示已经上电 */
    gpio_value = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6);
    if (gpio_value == 0)
    {
        power_on_system();       
    }
    else
    {
        reg_val[REQ_SHUTDOWN] = 0x55;    
    }
}

int ret_press_release = -1;
unsigned int end_time_value = 0;

/**
 * 下降沿触发
 * 按下70ms判做press, 连续高电平100ms,判做release
 */
void button_press_release(void)
{
    end_time_value = s_numOf100us;
    if (press_flag)
    {
        if ((GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7) > 0)) // release         
        {
            if (start_release)
            {
                start_release = 0;
                start_release_time = end_time_value;
            }

            ret_press_release = greater_times(start_release_time, end_time_value, 1000);                       
            if (ret_press_release)
            {
                press_flag = 0;
                start_release = 1; 
                press_trigger = 1; 
                printf("pp\n");
            }
        }
        else 
        {            
            start_release_time = end_time_value;
            start_release = 1;
        }

        if (greater_times(press_trigger_time, start_release_time, 700))  
        {
            if (short_press_flag)            
            {
                short_press_flag = 0;
                power_on_off();
            }
        }
    }
}
#endif

void pwr_control(void)
{	
	Gpio_Text();
	if ( 0x44 == reg_val[SHUTDOWN_REG])
	{
        power_off_system();
        reg_val[SHUTDOWN_REG] = 0x00;
        reg_val[REQ_SHUTDOWN] = 0x00;
	}
	if ( 0x55 == reg_val[REBOOT_REG])
	{	
		printf("****** restart system ******\n\r");
        /* 重启至少延迟5s后，上电 */
 //       hds3ss215_switch_pc();              // 视频掉电环回
        fan_setduty(0);    //先关风扇
        delay_ms(1000);
        GPIO_ResetBits(GPIOD, GPIO_Pin_6);    // 对整个系统下电
        led_ctl(BLUE_LED, LED_OFF);
        oled_clear_diplay();    
        delay_ms(1000);
        delay_ms(1000);
        delay_ms(1000);
        delay_ms(1000);
        delay_ms(1000);
 //       tmds_init();
        power_on_system();        
        reg_val[REBOOT_REG] = 0x00;
	}
}

/* poe reset */
void pse_reset(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);	

    GPIO_ResetBits(GPIOE, GPIO_Pin_2);
    delay_ms(5);
	GPIO_SetBits(GPIOE, GPIO_Pin_2);    
}

/* use polling to control power key */
#define LOW_LEVEL       0
#define HIGH_LEVEL      1
int first_down_flag             = 0;
//unsigned int first_down_time    = 0;
int first_up_flag               = 0;
unsigned int first_up_time      = 0;

/* 按下立刻开机，连续高电平500ms判为释放 */
static void detect_power_on(void)
{
    if (!first_down_flag) // 第一次按下
    {
        if (LOW_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))
        {
            first_down_flag = 1;
            //first_down_time = s_numOf100us;
            power_on_system();
            printf("aa power on\n\r");
        }
    }

    if (first_down_flag && (0 == first_up_flag))
    {
        if (LOW_LEVEL < GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7)) // 释放
        {
            first_up_flag = 1;
            first_up_time = s_numOf100us;
        }
    }

    if (first_up_flag)
    {
        if (greater_times(first_up_time, s_numOf100us, 5000))
        {
            first_down_flag = 0;
            //first_down_time = 0;    
            first_up_flag = 0;
            first_up_time = 0;
        }

        if (LOW_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))
        {
            first_up_flag = 0;
        }
    }

} 

int off_first_down_flag = 0;
unsigned int off_first_down_time = 0;
int off_first_up_flag = 0;
unsigned int off_first_up_time = 0;
int power_off_flag = 0;

/* 按下3s关机，连续高电平500ms判为释放 */
static void detect_power_off()
{
    if (!off_first_down_flag)    // 第一次按下
    {
        if (LOW_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))
        {
            off_first_down_flag = 1;
            off_first_down_time = s_numOf100us; // 记录按下时间
            printf("bb\n\r");
        }
    }

    if (off_first_down_flag && (0 == off_first_up_flag)) // 释放
    {
        if (LOW_LEVEL < GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))
        {
            off_first_up_flag = 1;
            off_first_up_time = s_numOf100us; // 记录释放时间
        }
    }

    if (off_first_up_flag) 
    {
        if (greater_times(off_first_up_time, s_numOf100us, 5000)) // high_level > 500ms: 释放
        {
            if ((0 == power_off_flag) && (greater_times(off_first_down_time, off_first_up_time, 50000)))
            {                
                reg_val[REQ_SHUTDOWN] = 0x55;
                //power_off_system();
                power_off_flag = 1;
                printf("cc\n\r");
            }
            off_first_down_flag = 0;
            off_first_down_time = 0;
            off_first_up_flag = 0;
            off_first_up_time = 0;
            power_off_flag = 0;
        }

        if (LOW_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))  // 抖动，重新记录释放时间
        {
            off_first_up_flag = 0;
        }
    }            

    if (off_first_down_flag && (0 == power_off_flag))    
    {
        if (greater_times(off_first_down_time, s_numOf100us, 50000))  // 一直按住，连续5s，请求关机
        {
            reg_val[REQ_SHUTDOWN] = 0x55;
           power_off_system();
           power_off_flag = 1;
           printf("dd\n\r");
        }
    }
}

void detect_power_pin(void)
{
    int gpio_value = -1;

    /* 高电平，表示已经上电 */
    gpio_value = GPIO_ReadOutputDataBit(GPIOD, GPIO_Pin_6);    
    if (((0 == gpio_value) || first_down_flag) && (0 == off_first_down_flag))
    {
        detect_power_on();
    }
    else if (((1 == gpio_value) || off_first_down_flag) && (0 == first_down_flag)) 
    {
        detect_power_off();
    }
} 

