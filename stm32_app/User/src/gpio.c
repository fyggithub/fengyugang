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
	
	GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;     // 推挽输出
    GPIO_Init(GPIOE, &GPIO_InitStructure);  

	GPIO_SetBits(GPIOE, GPIO_Pin_0);     //MIC1使能
	GPIO_SetBits(GPIOE, GPIO_Pin_1);     //MIC2使能
    GPIO_ResetBits(GPIOE, GPIO_Pin_3);
	GPIO_ResetBits(GPIOE, GPIO_Pin_4);
	GPIO_ResetBits(GPIOE, GPIO_Pin_5);
	
	GPIO_ResetBits(GPIOE, GPIO_Pin_8);
	GPIO_ResetBits(GPIOE, GPIO_Pin_9);
	GPIO_ResetBits(GPIOE, GPIO_Pin_10);

/*		GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_0;
		GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
		//GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;		// 置成上拉输入
		GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;		// 浮空输入
		GPIO_Init(GPIOD, &GPIO_InitStructure);	*/
}

void power_on_system(void)
{
	u8 pBuff[] = "****** power_on_system *******";
//	printf("****** power_on_system ******\n\r");
	Debug_String(pBuff,sizeof(pBuff) - 1);
	led_ctl(BLUE_LED, LED_ON);
	oled_clear_diplay();    
	oled_print_logo();

	GPIO_SetBits(GPIOD, GPIO_Pin_6);	 // 对整个系统上电
	GPIO_SetBits(GPIOE, GPIO_Pin_2); 	 // poe power on
	GPIO_SetBits(GPIOA, GPIO_Pin_11);  //PCIE_USB1复位上电
	GPIO_SetBits(GPIOA, GPIO_Pin_12);  //PCIE_USB2复位上电
	GPIO_SetBits(GPIOE, GPIO_Pin_7);   //WIFI复位上电
	GPIO_SetBits(GPIOD, GPIO_Pin_3);   //LAN SWITCH复位上电
	GPIO_SetBits(GPIOE, GPIO_Pin_0);   //MIC1上电
	GPIO_SetBits(GPIOE, GPIO_Pin_1);   //MIC2上电
	
	delay_ms(1000);
	reboot_flag = 1;  
	clean_uart_buf_time = s_numOf100us;
	Clear_Time_Flag();
}

static void power_off_system(void)
{
	u8 pBuff[] = "****** power_off_system *******";
	
//	printf("****** power_off_system ******\n");	
	fan_setduty(FAN_STOP);    										//先关风扇
	delay_ms(1000);
	GPIO_ResetBits(GPIOD, GPIO_Pin_6);    // 对整个系统下电

	led_ctl(BLUE_LED, LED_OFF);
	oled_clear_diplay(); 
	GPIO_ResetBits(GPIOE, GPIO_Pin_2); // poe power down
	GPIO_ResetBits(GPIOA, GPIO_Pin_11);  //PCIE_USB1复位掉电
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);  //PCIE_USB2复位掉电
	GPIO_ResetBits(GPIOE, GPIO_Pin_7);   //WIFI复位掉电
	GPIO_ResetBits(GPIOD, GPIO_Pin_3);   //LAN SWITCH复位掉电
	GPIO_ResetBits(GPIOE, GPIO_Pin_0);   //MIC1掉电
	GPIO_ResetBits(GPIOE, GPIO_Pin_1);   //MIC2掉电
	
	delay_ms(1000);
	delay_ms(1000);
//	delay_ms(1000);
	
	Debug_String(pBuff,sizeof(pBuff) - 1);
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
	u8 pBuff[] = "*** restart system ***";
	if ( 0x44 == reg_val[REQ_SHUTDOWN])
	{
        power_off_system();
        reg_val[REQ_SHUTDOWN] = 0x00;
	}
	if ( 0x55 == reg_val[REBOOT_REG])
	{	
//		printf("****** restart system ******\n\r");/* 重启至少延迟5s后，上电 */        
		Debug_String(pBuff,sizeof(pBuff) - 1);
//      hds3ss215_switch_pc();              // 视频掉电环回
        fan_setduty(FAN_STOP);    //先关风扇
        delay_ms(1000);
        GPIO_ResetBits(GPIOD, GPIO_Pin_6);    // 对整个系统下电
        led_ctl(BLUE_LED, LED_OFF);
        oled_clear_diplay();    
        delay_ms(1000);
        delay_ms(1000);
//        delay_ms(1000);
//        delay_ms(1000);
//        delay_ms(1000);
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
//	u8 pBuff[] = "keydown_on";
    if (!first_down_flag) // 第一次按下
    {
        if (LOW_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))   //按键被按下
        {
            first_down_flag = 1;
            first_down_time = s_numOf100us;
//			Debug_String(pBuff,sizeof(pBuff) - 1);
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
//	u8 pBuff[] = "keydown_off";
    if (!off_first_down_flag)    // 第一次按下
    {
        if (LOW_LEVEL == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))
        {
            off_first_down_flag = 1;
            off_first_down_time = s_numOf100us; // 记录按下时间
//			Debug_String(pBuff,sizeof(pBuff) - 1);
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

void Mic_Set(int module,int val)
{
	int mod,io_set;
	
	mod = module;
	io_set = val & 0x07;
	
	switch(mod)
	{
		case 1:{
				if(io_set & 0x01){									
					GPIO_SetBits(GPIOE, GPIO_Pin_8);
				}
				else{				
					GPIO_ResetBits(GPIOE, GPIO_Pin_8);
				}
				
				if(io_set & 0x02){				
					GPIO_SetBits(GPIOE, GPIO_Pin_9);
				}
				else{				
					GPIO_ResetBits(GPIOE, GPIO_Pin_9);
				}
				
				if(io_set & 0x04){
					GPIO_SetBits(GPIOE, GPIO_Pin_10);				
				}
				else{				
					GPIO_ResetBits(GPIOE, GPIO_Pin_10);
				}
			}break;
		case 2:{
				if(io_set & 0x01){									
					GPIO_SetBits(GPIOE, GPIO_Pin_3);
				}
				else{				
					GPIO_ResetBits(GPIOE, GPIO_Pin_3);
				}
				
				if(io_set & 0x02){				
					GPIO_SetBits(GPIOE, GPIO_Pin_4);
				}
				else{				
					GPIO_ResetBits(GPIOE, GPIO_Pin_4);
				}
				
				if(io_set & 0x04){
					GPIO_SetBits(GPIOE, GPIO_Pin_5);				
				}
				else{				
					GPIO_ResetBits(GPIOE, GPIO_Pin_5);
				}			
			}break;
		default:break;
	}
}

void Update_Mic_Gain(void)
{
	switch(reg_val[MIC1_ENABLE])
	{
		case 1:{
				reg_val[MIC1_ENABLE] = 0;
				GPIO_SetBits(GPIOE, GPIO_Pin_0);
//				printf("MIC1 enable!\n");
			}break;    //MIC1上电
		case 2:{
				reg_val[MIC1_ENABLE] = 0;
				GPIO_ResetBits(GPIOE, GPIO_Pin_0);
//				printf("MIC1 disable!\n");
			}break;    //MIC1掉电
		default:break;
	}
	switch(reg_val[MIC2_ENABLE])
	{
		case 1:{
				reg_val[MIC2_ENABLE] = 0;
				GPIO_SetBits(GPIOE, GPIO_Pin_1);
//				printf("MIC2 enable!\n");
			}break;     //MIC2上电
		case 2:{
				reg_val[MIC2_ENABLE] = 0;
				GPIO_ResetBits(GPIOE, GPIO_Pin_1);
//				printf("MIC2 disable!\n");
			}break;    //MIC2掉电
		default:break;
	}
	
	if(1 == reg_val[MIC1_TYPE])
	{
		reg_val[MIC1_TYPE] = 0;
		Mic_Set(1,reg_val[MIC1_CTL]);
//		printf("MIC1 Ctl : %d !\n",reg_val[MIC1_CTL]);
	}
	if(1 == reg_val[MIC2_TYPE])
	{
		reg_val[MIC2_TYPE] = 0;
		Mic_Set(2,reg_val[MIC2_CTL]);
//		printf("MIC2 Ctl : %d !\n",reg_val[MIC2_CTL]);
	}
}

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

