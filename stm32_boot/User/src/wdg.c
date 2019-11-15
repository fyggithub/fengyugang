#include "wdg.h"

void IWDG_Init(u8 prer,u16 rlr) 
{	
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //ʹ�ܶԼĴ���IWDG_PR��IWDG_RLR��д����
	
	IWDG_SetPrescaler(prer);  //����IWDGԤ��Ƶֵ:����IWDGԤ��ƵֵΪ64
	
	IWDG_SetReload(rlr);  //����IWDG��װ��ֵ
	
	IWDG_ReloadCounter();  //����IWDG��װ�ؼĴ�����ֵ��װ��IWDG������
	
	IWDG_Enable();  //ʹ��IWDG
}

/*ι�������Ź�*/
void IWDG_Feed(void)
{   
 	IWDG_ReloadCounter();//���ؼ���ֵ									   
}


/*����WWDG������������ֵ,Ĭ��Ϊ���.*/
u8 WWDG_CNT = 0x7f; 

/*
 * ��ʼ�����ڿ��Ź� 	
 * tr   :T[6:0],������ֵ 
 * wr   :W[6:0],����ֵ 
 * fprer:��Ƶϵ����WDGTB��,�����2λ��Ч 
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

/* ������WWDG��������ֵ */
void WWDG_Set_Counter(u8 cnt)
{
    WWDG_Enable(cnt);//ʹ�ܿ��Ź� ,	���� counter .	 
}

void WWDG_NVIC_Init()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;   //��ռ2�������ȼ�3����2	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	 //��ռ2�������ȼ�3����2	
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void WWDG_IRQHandler(void)
{
    /* �������˾��,���ڿ��Ź���������λ */
	WWDG_SetCounter(WWDG_CNT);	  
	WWDG_ClearFlag(); //�����ǰ�����жϱ�־λ
}

