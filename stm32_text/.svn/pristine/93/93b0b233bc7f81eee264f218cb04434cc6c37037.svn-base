#ifndef __WDG_H
#define __WDG_H
#include "stm32f10x.h"

#define TR_MASK		0xFF
#define WR_VALUE	0x5F
#define PRER_VALUE	6
#define RLR_VALUE	625

extern void IWDG_Init(u8 prer,u16 rlr);
extern void IWDG_Feed(void);

extern void WWDG_Init(u8 tr,u8 wr,u32 fprer);//初始化WWDG
extern void WWDG_Set_Counter(u8 cnt);       //设置WWDG的计数器
extern void WWDG_NVIC_Init(void);
#endif
