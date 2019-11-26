#include "timer.h"
#include "gpio.h"
#include "debug_usart.h"
#include "wdg.h"
#include "led.h"

extern u8 led_blink[3];
//unsigned int s_numOf40us = 0;
unsigned int s_numOf100us = 0;
unsigned int s_num = 0;
unsigned int s_numof1s = 0;
unsigned int tim4_count = 0;
unsigned int iwdg_count = 0;

/*
 * ͨ�ö�ʱ��3�жϳ�ʼ��
 * ����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
 * arr���Զ���װֵ��
 * psc��ʱ��Ԥ��Ƶ��
 * ����ʹ�õ��Ƕ�ʱ��3!
 * Tout= ((arr+1)*(psc+1))/Tclk 
 * ��������Ϊ0.1ms   prescaler: 100-1, period: 72-1
 */
void TIM3_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; 					//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 					//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 	//����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 			//����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); 					//ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  			//TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  	//��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; 			//�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 			//IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  							//��ʼ��NVIC�Ĵ���

	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx					 
}

/* ��ʱ��3�жϷ������ */
void TIM3_IRQHandler(void)   //TIM3�ж�
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx�����жϱ�־ 
        s_num++;
        s_numOf100us++;
        if (MAX_VALUE_TIME == s_numOf100us)
        {
			s_numOf100us = 0;
        }	

        if(s_num >= 10000)
        {
            s_numof1s++;
            s_num = 0;
        }
        if(MAX_VALUE_TIME / 2 == s_numof1s)
        {
            s_numof1s = 0;
        }
            
        if (greater_times(iwdg_count, s_numOf100us, 30000))
        {
            IWDG_Feed();
            iwdg_count = s_numOf100us;
        }
    }
}

/** TIM4 PWM���ֳ�ʼ�� 
 * PWM�����ʼ��
 * arr���Զ���װֵ
 * psc��ʱ��Ԥ��Ƶ��
 * Tout= ((arr+1)*(psc+1))/Tclk 
 */
void tim4_pwn_init(u16 arr, u16 psc)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE); 
    GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); //Timer4 ��ӳ��  TIM4_CH2->PD13  

    //���ø�����Ϊ�����������,��� TIM4 CH2 �� PWM ���岨�� GPIOD.13
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 					//TIM_CH2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 			//�����������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure); 						//��ʼ�� GPIO    

    //��ʼ��TIM4
    TIM_TimeBaseStructure.TIM_Period = arr; 					//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
    TIM_TimeBaseStructure.TIM_Prescaler = psc;					//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ 
    //TIM_TimeBaseStructure.TIM_Period = 9; 					//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
    //TIM_TimeBaseStructure.TIM_Prescaler = 71;					//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//����ʱ�ӷָ�:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//TIM���ϼ���ģʽ
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);    

    /* TIM1_OC1ģ������(����1ͨ��ռ�ձ�)  */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
    TIM_OCInitStructure.TIM_Pulse = 60;	
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);    

    TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable); 
    //TIM_ARRPreloadConfig(TIM4, ENABLE); 

    TIM_CtrlPWMOutputs(TIM4, ENABLE);	// TIM1_OCͨ�����PWM(һ��Ҫ��)
    TIM_Cmd(TIM4, ENABLE);	        
}

/* ����TIM4�������PWMʱ�õ���I/O */
static void TIM4_GPIO_Config(void) 
{
  GPIO_InitTypeDef GPIO_InitStructure;

 // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); 
  GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); //Timer4 ��ӳ��  TIM4_CH2->PD13 

  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		    // �����������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}

/* ����Ƕ�������жϿ�����NVIC */
static void NVIC_Config_PWM(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Configure one bit for preemption priority */
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  /* ����TIM3_IRQ�ж�Ϊ�ж�Դ */
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

static void TIM4_Mode_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;																				

    /* ����TIM4CLK ʱ��Ϊ72MHZ */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);        //ʹ��TIM4ʱ��

    /* ������ʱ������ */		             
    TIM_TimeBaseStructure.TIM_Period = ARR_TIM4;       		    //����ʱ����0������ARR_TIM4����ΪARR_TIM4�Σ�Ϊһ����ʱ����
    TIM_TimeBaseStructure.TIM_Prescaler = PSC_TIM4;	    	    //����Ԥ��Ƶ��
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;	//����ʱ�ӷ�Ƶϵ��������Ƶ(�����ò���)
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //���ϼ���ģʽ
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* PWMģʽ���� */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	    		//����ΪPWMģʽ1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//ʹ�����
    TIM_OCInitStructure.TIM_Pulse = 0;								//���ó�ʼPWM������Ϊ0	
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;  	    //����ʱ������ֵС��CCR1_ValʱΪ�ߵ�ƽ

    TIM_OC2Init(TIM4, &TIM_OCInitStructure);	 					//ʹ��ͨ��2
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);				//ʹ��Ԥװ��	

    TIM_OC3Init(TIM4, &TIM_OCInitStructure);	 					//ʹ��ͨ��3
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);				//ʹ��Ԥװ��	

    TIM_OC4Init(TIM4, &TIM_OCInitStructure);	 					//ʹ��ͨ��4
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);				//ʹ��Ԥװ��	
    
    TIM_ARRPreloadConfig(TIM4, ENABLE);			 					//ʹ��TIM4���ؼĴ���ARR    
    //TIM_CtrlPWMOutputs(TIM4,ENABLE);                              //����TIM4 ��PWM ���Ϊʹ�� 

    /* TIM4 enable counter */
    TIM_Cmd(TIM4, ENABLE);                   						//ʹ�ܶ�ʱ��4
    TIM_ITConfig(TIM4,TIM_IT_Update, ENABLE);						//ʹ��update�ж�	
    NVIC_Config_PWM();												//�����ж����ȼ�			
    led_ctl(BLUE_LED, LED_ON);                                      // �ϵ磬��Ĭ����ɫ

    //TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Disable);				//disableԤװ��	
    //TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Disable);				//disableԤװ��	
    //TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Disable);				//disableԤװ��	
}

/* TIM4 �����Ƴ�ʼ�� */
void TIM4_Breathing_Init(void)
{
	TIM4_GPIO_Config();
	TIM4_Mode_Config();	
}

int ledpwmval = 500;
u8 dir = 0;
int led_enable_r = 0;
int led_enable_b = 0;
int led_enable_g = 0;

/* 22ms */
void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {       
        if (led_enable_r)
        {
            TIM4->CCR2 = ledpwmval;    //����PWM���޸Ķ�ʱ���ıȽϼĴ���ֵ
            
            if (dir)
            {                    
                ledpwmval += 2;
            }
            else 
            {
                ledpwmval -= 2;
            }

            if (ledpwmval > 500)
                dir = 0;
            if (0 == ledpwmval)
                dir = 1;     
        }

        if (led_enable_g)
        {
            TIM4->CCR3 = ledpwmval;    //����PWM���޸Ķ�ʱ���ıȽϼĴ���ֵ
            
            if (dir)
            {                    
                ledpwmval += 2;
            }
            else 
            {
                ledpwmval -= 2;
            }

            if (ledpwmval > 500)
                dir = 0;
            if (0 == ledpwmval)
                dir = 1;     
        }

        if (led_enable_b)
        {
            TIM4->CCR4 = ledpwmval;    //����PWM���޸Ķ�ʱ���ıȽϼĴ���ֵ
            
            if (dir)
            {                    
                ledpwmval += 2;
            }
            else 
            {
                ledpwmval -= 2;
            }

            if (ledpwmval > 500)
                dir = 0;
            if (0 == ledpwmval)
                dir = 1;     
        }            

        led_blink_ir_timer();    
        led_loop_blink_timer();
        led_breathing_test();    // for test
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);    //����Ҫ����жϱ�־λ
    }
}

