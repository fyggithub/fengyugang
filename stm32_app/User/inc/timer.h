#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

#define PRESCALER_TIM3		99		/* 设置自动重装载寄存器周期的值 */
#define PERIOD_TIM3			71		/* 设置时钟频率除数的预分频值 */
#define PRESCALER_TIM4		500		/* 设置自动重装载寄存器周期的值 */
#define PERIOD_TIM4			3200	/* 设置时钟频率除数的预分频值 */
#define ARR_TIM4            500
#define PSC_TIM4            1700
#define LED_ON_T            0
#define LED_OFF_T           (ARR_TIM4+4)


void TIM1_InitPwm(void);
void TIM1_SetPwm(u16 pulse);
void TIM3_Int_Init(u16 arr, u16 psc);
void TIM3_IRQHandler(void);
void Wdg_Feed(void);

void tim4_init(u16 prescaler, u16 period);
void tim4_pwn_init(u16 arr, u16 psc);

void TIM4_Breathing_Init(void);


#endif
