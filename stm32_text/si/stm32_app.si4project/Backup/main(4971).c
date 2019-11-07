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

extern unsigned int s_numOf100us;
extern u8 camera_ir_flag;       /* camera ir 干扰标志 */
extern unsigned int s_numof1s;  /* 秒记时2^31s */
//unsigned int cur_time;
unsigned int camera1_time;      /* camera1当前时间 */
unsigned int camera2_time;      /* camera2当前时间 */
unsigned char IR_NOISE_DETECT;   /* 检测到干扰后的中断屏蔽时间 s */


// while 循环，一次200us
int main(void)  
{
    int count = 0;
    
    sys_init();	

    cpu_power_init();
    delay_ms(100);

    printf("enter app\n");

    while(1)
    {
        count++;
        clean_uart_buf();
        if (count == 2000000)
        {
            printf("boot\n");
            count = 0;
        }
    }


#if 0
    int camera1_time_cnt = 0;
    int camera2_time_cnt = 0;
    IR_NOISE_DETECT = 3;
    
	sys_init();		
	cpu_power_init();
    delay_ms(100);
    hds3ss215_switch_rk3339();
    printf("enter app\n");

    while(1)
	{			   
		hostBoardProc();
		sys_update();
        IT6662_Main();

        camera1_time_cnt = s_numof1s - camera1_time;
        if(camera1_time_cnt < 0)
            camera1_time_cnt = 0 - camera1_time_cnt;
        if(camera1_time_cnt >= IR_NOISE_DETECT) //IR_NOISE_DETECT秒后打开中断 lq
        {
            camera1_time = s_numof1s;
            if(camera_ir_flag & 0x10)
            {
                camera_ir_flag &= ~ 0x10;
                //开中断
                if (0 == (EXTI->IMR & EXTI_Line6))
                {
                	//printf("open camrea1 ir %d \n", s_numof1s);
                    EXTI->IMR |= EXTI_Line6;
                }
            }
        }

        camera2_time_cnt = s_numof1s - camera2_time;
        if(camera2_time_cnt < 0)
            camera2_time_cnt = 0 - camera2_time_cnt;
        if(camera2_time_cnt >= IR_NOISE_DETECT)
        {
            camera2_time = s_numof1s;
            if(camera_ir_flag & 0x20)
            {
                camera_ir_flag &= ~ 0x20;
                //开中断
                if (0 == (EXTI->IMR & EXTI_Line4))
                {
                //printf("open camrea2 ir %d \n", s_numof1s);
                EXTI->IMR |= EXTI_Line4;
                }
            }  
        }
	}

    #endif
}
