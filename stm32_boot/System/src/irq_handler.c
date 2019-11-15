#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"

#if 0
static void IRQ_GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); 

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
    GPIO_Init(GPIOD, &GPIO_InitStructure);  
}

static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
	
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 

    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);  
}

static void EXTI_Configuration(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;  

	EXTI_ClearITPendingBit(EXTI_Line3);//清空中断标志
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource3); //选择管脚
	EXTI_InitStructure.EXTI_Line = EXTI_Line3;  //中断线路3
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //设置中断请求，非事件请求
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//下降沿触发  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;  
    EXTI_Init(&EXTI_InitStructure);  
}

void irq_Init(void)
{
    IRQ_GPIO_Configuration();
	NVIC_Configuration();
	EXTI_Configuration();
}

//IIC从设备主动发起数据请求中断
void EXTI3_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line3) != RESET)  
    {  
 

         
        EXTI_ClearITPendingBit(EXTI_Line3);  
    }  
}
#endif
