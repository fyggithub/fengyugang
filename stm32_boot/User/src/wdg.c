#include "wdg.h"

void IWDG_Init(u8 prer,u16 rlr) 
{	
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //使能对寄存器IWDG_PR和IWDG_RLR的写操作
	
	IWDG_SetPrescaler(prer);  //设置IWDG预分频值:设置IWDG预分频值为64
	
	IWDG_SetReload(rlr);  //设置IWDG重装载值
	
	IWDG_ReloadCounter();  //按照IWDG重装载寄存器的值重装载IWDG计数器
	
	IWDG_Enable();  //使能IWDG
}

/*喂独立看门狗*/
void IWDG_Feed(void)
{   
 	IWDG_ReloadCounter();//重载计数值									   
}


/*保存WWDG计数器的设置值,默认为最大.*/
u8 WWDG_CNT = 0x7f; 

/*
 * 初始化窗口看门狗 	
 * tr   :T[6:0],计数器值 
 * wr   :W[6:0],窗口值 
 * fprer:分频系数（WDGTB）,仅最低2位有效 
 * Fwwdg=PCLK1/(4096*2^fprer). 
 */
void WWDG_Init(u8 tr, u8 wr, u32 fprer)
{ 	
	WWDG_CNT = tr & WWDG_CNT;
	WWDG_SetPrescaler(fprer);
	WWDG_SetWindowValue(wr);
	WWDG_Enable(WWDG_CNT);
	WWDG_ClearFlag();
	WWDG_NVIC_Init();
	WWDG_EnableIT();
} 

/* 重设置WWDG计数器的值 */
void WWDG_Set_Counter(u8 cnt)
{
    WWDG_Enable(cnt);//使能看门狗 ,	设置 counter .	 
}

void WWDG_NVIC_Init()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;   //抢占2，子优先级3，组2	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	 //抢占2，子优先级3，组2	
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void WWDG_IRQHandler(void)
{
    /* 当禁掉此句后,窗口看门狗将产生复位 */
	WWDG_SetCounter(WWDG_CNT);	  
	WWDG_ClearFlag(); //清除提前唤醒中断标志位
}

