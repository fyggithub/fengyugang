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
#include "process.h"

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

typedef enum{
	DOWN_STATUS = 0,
	READY1_STATUS,
	READY2_STATUS,
	UP_STATUS,
	MAX_STATUS,
}KEY_STATUS;

KEY_STATUS gStatus = DOWN_STATUS;

/* begin 5728 上下电控制 */
extern unsigned int s_numOf100us;
extern unsigned char reg_val[I2C_REG_NUM];
/* end 5728 上下电控制 */

int off_first_down_flag = 0;
int off_first_up_flag = 0;
unsigned int off_first_down_time = 0;
unsigned int off_first_up_time = 0;

int first_down_flag             = 0;
int first_up_flag               = 0;
unsigned int first_down_time    = 0;
unsigned int first_up_time      = 0;

int auto_shutdown_flag          = 0;
int auto_start_counttime        = 0;

int reboot_flag = 0;
unsigned int clean_uart_buf_time = 0;
int key_shutdown_flag = 0;


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
	Clear_Time_Flag();
}

static void power_off_system(void)
{
	printf("****** power_off_system ******\n");
	fan_setduty(FAN_STOP);    										//先关风扇
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

void pwr_control(void)
{	
//  Gpio_Text();
	if ( 0x44 == reg_val[REQ_SHUTDOWN])
	{
        power_off_system();
        reg_val[REQ_SHUTDOWN] = 0x00;
	}
	if ( 0x55 == reg_val[REBOOT_REG])
	{	
		printf("****** restart system ******\n\r");/* 重启至少延迟5s后，上电 */        
 //       hds3ss215_switch_pc();              // 视频掉电环回
        fan_setduty(FAN_STOP);    //先关风扇
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
/* 按下立刻开机，连续高电平500ms判为释放 */
static void detect_power_on(void)
{
    if (!first_down_flag) // 第一次按下
    {
        if (LOW_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))   //按键被按下
        {
            first_down_flag = 1;
            first_down_time = s_numOf100us;
			printf("aa\n\r");
        }
    }

    if (first_down_flag && (0 == first_up_flag))
    {
        if (HIGH_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7)) // 释放
        {
            first_up_flag = 1;
            first_up_time = s_numOf100us;
        }
    }

    if (first_up_flag)
    {
        if (greater_times(first_down_time, first_up_time, 5000))   //低电平如果时间有500ms，则认为被按下，
        {
			power_on_system();              //系统开机
			
            first_down_flag = 0;
			first_down_time = 0;
            first_up_flag = 0;
            first_up_time = 0;
			
			auto_shutdown_flag = 0;
			auto_start_counttime = 0;
        }

        if (LOW_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))  //去除毛刺
        {
            first_up_flag = 0;
        }
    }
} 

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
        if (HIGH_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))
        {
            off_first_up_flag = 1;
            off_first_up_time = s_numOf100us; // 记录释放时间
        }
    }

    if (off_first_up_flag) 
    {
		if(greater_times(off_first_up_time,s_numOf100us,5000))                //没有到5s，按键就被松开了
		{
			off_first_down_flag = 0;
			off_first_down_time = 0;
			off_first_up_flag = 0;
			off_first_up_time = 0;
		}

		if (greater_times(off_first_down_time, off_first_up_time, 50000))  // 一直按住，连续5s，请求关机
        {
			key_shutdown_flag = 1;
//			Send_To_Request(SYS_KEY_SHUTDOWN_CMD);                             //向上位机请求关机
//			power_off_system();
			off_first_down_flag = 0;
			off_first_down_time = 0;
			off_first_up_flag = 0;
			off_first_up_time = 0;
			
			auto_shutdown_flag = 1;
			auto_start_counttime = s_numOf100us;
        }
		
		if (LOW_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))  // 抖动，重新记录释放时间
		{
			off_first_up_flag = 0;
		}
    } 

	if(1 == auto_shutdown_flag)
	{
		if (greater_times(auto_start_counttime, s_numOf100us, 50000))
		{
			auto_shutdown_flag = 0;
			auto_start_counttime = 0;
			key_shutdown_flag = 0;
			if (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) == HIGH_LEVEL)           //如果关机了，则不管
			{							
				reg_val[REQ_SHUTDOWN] = 0x44;         
			}
		}
	}
		
}

void Get_Shutdown_Value(void)
{
	if(key_shutdown_flag == 1)
	{
		key_shutdown_flag = 0;
		Send_To_Request(SYS_KEY_SHUTDOWN_CMD);                             //向上位机请求关机		
	}
	else
	{
		Send_To_Request(SYS_WATCH_KEY_SHUTDOWN_CMD);
	}
}

void Clear_Time_Flag(void)
{
	auto_shutdown_flag = 0;
	auto_start_counttime = 0;
}
/* 按下立刻开机，连续高电平500ms判为释放 */
/*static void detect_power_on(void)
{
	static unsigned int down_time = 0,up_time = 0;
	switch(gStatus)
	{
		case DOWN_STATUS:
				{
					if (LOW_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))
					{
						gStatus = READY1_STATUS;
						down_time = s_numOf100us;
					}
				}break;	
		case READY1_STATUS:
				{
					if (HIGH_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))
					{
						up_time = s_numOf100us;
						gStatus = READY2_STATUS;
					}
				}break;
		case READY2_STATUS:
				{					
					if (greater_times(down_time, up_time, 3000))
					{
						gStatus = UP_STATUS;
					}
					
					if (greater_times(up_time, s_numOf100us, 3000))
					{
						gStatus = MAX_STATUS;
					}
					
					if (LOW_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))             //抖动产生，重新读取电平
					{
						gStatus = READY1_STATUS;						
					}
				}break;
		case UP_STATUS:
				{
					power_on_system();
					gStatus = MAX_STATUS;
				}break;
		case MAX_STATUS:gStatus = DOWN_STATUS;break;	
		default:break;
	}
}
*/
/* 按下5s关机，连续高电平300ms判为释放 */
/*static void detect_power_off(void)
{
	static unsigned int down_time = 0,up_time = 0,auto_time = 0;
	switch(gStatus)
	{
		case DOWN_STATUS:
				{
					if (LOW_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))
					{
						down_time = s_numOf100us;
						gStatus = READY1_STATUS;						
					}
				}break;	
		case READY1_STATUS:
				{
					if (HIGH_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))
					{
						up_time = s_numOf100us;
						gStatus = READY2_STATUS;
					}
				}break;
		case READY2_STATUS:
				{							
					if (greater_times(down_time, up_time, 50000))
					{
						Send_To_Request(SYS_SHUTDOWN_CMD);                                 //发送关机请求
						auto_time = s_numOf100us;
						gStatus = MAX_STATUS;
					}
					
					if (greater_times(up_time, s_numOf100us, 3000))
					{
						gStatus = MAX_STATUS;
					}
					
					if (LOW_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))             //抖动产生，重新读取电平
					{
						gStatus = READY1_STATUS;						
					}
				}break;
		case UP_STATUS:break;
		case MAX_STATUS:
				{
					if (greater_times(auto_time, s_numOf100us, 50000))
					{
						if (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) == HIGH_LEVEL)        //如果5s过后还没有关机，则强制关机
						{							
							reg_val[REQ_SHUTDOWN] = 0x44;							
						}						
					}	
					gStatus = DOWN_STATUS;
				}break;				
		default:break;
	}
}
*/

void detect_power_pin(void)
{
    int gpio_value = -1;
   
    gpio_value = GPIO_ReadOutputDataBit(GPIOD, GPIO_Pin_6);    /* 高电平，表示已经上电 */
    if (((0 == gpio_value) || first_down_flag) && (0 == off_first_down_flag))
    {
        detect_power_on();
    }
    else if (((1 == gpio_value) || off_first_down_flag) && (0 == first_down_flag)) 
    {
        detect_power_off();
    }
} 

