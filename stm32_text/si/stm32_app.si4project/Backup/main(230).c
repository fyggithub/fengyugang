#include "common_init.h"
#include "process.h"
#include "delay.h"
#include "sys.h"
#include "gpio.h"
#include "debug_usart.h"
#include "wdg.h"
#include "lm75.h"
#include "ds1339c.h"
#include "ir.h"
#include "tmds181.h"
#include "fan.h"
#include "led.h"
#include "timer.h"
#include "oled.h"
#include "stm32f10x_i2c.h"
#include "myiic.h"
#include "usart.h"



// while 循环，一次200us
int main(void)  
{
    //int camera1_time_cnt = 0;
    //int camera2_time_cnt = 0;
    //IR_NOISE_DETECT = 3;
   	int count = 0; 
	sys_init();	

    oled_initDev();


   //cpu_power_init();
    delay_ms(100);
    //hds3ss215_switch_rk3339();
    printf("enter app\n");

    while(1)
	{			   
	    count++;
        if (count == 2000000)
        {
        	printf("boot\n");
        	count = 0;
        }
	}
}
