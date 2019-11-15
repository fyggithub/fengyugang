#include "sys.h"
#include "delay.h"
#include "debug_usart.h"
#include "irq_handler.h"
#include "wdg.h"
#include "usart.h"
#include "gpio.h"
#include "myiic.h"

extern void cpu_power_init(void);

static void RCC_Configuration(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG | RCC_APB1Periph_USART2 | 
		                   RCC_APB1Periph_USART3 | RCC_APB1Periph_I2C2 | RCC_APB1Periph_UART4, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC 
            | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_USART1, ENABLE);
}

void sys_init(void)
{
    delay_init();
    RCC_Configuration(); 	
	/* WWDG_Init(0x7F, 0x5F, WWDG_Prescaler_8); */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	
    GPIO_Configuration();
    cpu_power_init();
    duart_init(115200);
    uart4_init(115200);
	/* 5728一上电，uart4有时会传一个数据到stm32.
	 * 这里先让5728跑起来，然后清除这个数据         */     
    delay_ms(30);		// 
    uart4_clean_buf(); // 清除uart4 buffer, 以防5728一上电，传入无效数据
    i2c_init();
}

