#include "sys.h"
#include "stm32f10x_gpio.h"
#include "gpio.h"

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*œµÕ≥∞ÂGPIO≈‰÷√*/  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOD, GPIO_Pin_10);
    GPIO_ResetBits(GPIOD, GPIO_Pin_8);
    GPIO_ResetBits(GPIOD, GPIO_Pin_9);
   // GPIO_ResetBits(GPIOD, GPIO_Pin_6);
    /*÷˜øÿ∞ÂGPIO≈‰÷√*/  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    GPIO_ResetBits(GPIOB, GPIO_Pin_8);
    GPIO_ResetBits(GPIOB, GPIO_Pin_9);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 
        | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOE, GPIO_Pin_6);
		
}

void Gpio_setPb14(void)      
{
    GPIO_SetBits(GPIOB, GPIO_Pin_14);
}

void Gpio_allBoardRest(void)
{
    GPIO_ResetBits(GPIOE, GPIO_Pin_8);
	GPIO_ResetBits(GPIOE, GPIO_Pin_6);
	GPIO_ResetBits(GPIOE, GPIO_Pin_7);
	GPIO_ResetBits(GPIOE, GPIO_Pin_9);
	GPIO_ResetBits(GPIOE, GPIO_Pin_11);
	GPIO_ResetBits(GPIOE, GPIO_Pin_12);
	GPIO_ResetBits(GPIOE, GPIO_Pin_13);
	//GPIO_SetBits(GPIOE, GPIO_Pin_14);
    delay_ms(100);
    GPIO_SetBits(GPIOE, GPIO_Pin_8);
    GPIO_SetBits(GPIOE, GPIO_Pin_6);
	GPIO_SetBits(GPIOE, GPIO_Pin_7);
	GPIO_SetBits(GPIOE, GPIO_Pin_9);
	GPIO_SetBits(GPIOE, GPIO_Pin_11);
	GPIO_SetBits(GPIOE, GPIO_Pin_12);
	GPIO_SetBits(GPIOE, GPIO_Pin_13);
//	GPIO_ResetBits(GPIOE, GPIO_Pin_14);
}


