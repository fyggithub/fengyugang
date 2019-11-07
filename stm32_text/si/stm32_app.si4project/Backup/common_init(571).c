#include "sys.h"
#include "delay.h"
#include "debug_usart.h"
#include "irq_handler.h"
#include "lcd.h"
#include "wdg.h"
#include "usart.h"
#include "myiic.h"
#include "spi.h"
#include "gpio.h"
#include "timer.h"
#include "ds1339c.h"
#include "ir.h"
#include "fan.h"
#include "tmds181.h"
#include "led.h"
#include "oled.h"
#include "process.h"
#include "led.h"

static void RCC_Configuration(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG | RCC_APB1Periph_USART2 | RCC_APB1Periph_USART3 
            | RCC_APB1Periph_UART4 | RCC_APB1Periph_UART5 | RCC_APB1Periph_TIM3 
            | RCC_APB1Periph_SPI2 | RCC_APB1Periph_I2C2, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB 
            | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
}

void sys_init(void)
{
    delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
    RCC_Configuration();        

#if 0
    duart_init(BAUD_115200);	

    TIM3_Int_Init(PRESCALER_TIM3, PERIOD_TIM3);	
    IWDG_Init(PRER_VALUE, RLR_VALUE); // 4s喂狗
    WWDG_Init(TR_MASK, WR_VALUE, WWDG_Prescaler_8); // 4s喂狗

    hd3ss215_init();                    // 视频信号掉电环回
          
	/* 5728一上电，uart4有时会传一个数据到stm32.
	 * 这里先让5728跑起来，然后清除这个数据 */
    delay_ms(30);		
    uart4_clean_buf(); // 清除uart4 buffer, 以防5728一上电，传入无效数据
    uart4_init(36, BAUD_115200);		// AM5728与单片机间的串口，量产后可用于升级单片机
	i2c_init();
#if 1
    
	tmds_init();
	ir_init();                          // 红外初始化
	ds1339c_init();						// 时钟	
	TIM4_Breathing_Init();							// 状态灯
	oled_initDev();                     // oled屏幕初始化
	fan_init(FAN_ARR, FAN_PRESCALER);   // 风扇初始化  	
	Proc_setVer();
    pse_reset();
#endif
#endif

}
