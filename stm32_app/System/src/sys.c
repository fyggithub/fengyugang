#include "sys.h"
#include "debug_usart.h"

extern __INLINE void NVIC_SystemReset(void);


#if 1
typedef  void (*IapFun)(void); //����һ���������͵Ĳ���

static IapFun JumpToApp; 

/*
*THUMBָ�֧�ֻ������
*�������·���ʵ��ִ�л��ָ��WFI 
*/ 
void WFI_SET(void)
{
	__ASM volatile("wfi");		  
}

/*�ر������ж�*/
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}
/*���������ж�*/
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}


/*
*����ջ����ַ
*addr:ջ����ַ
*/
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 //set Main Stack value
    BX r14
}
#endif

/*��ת��Ӧ�ó��� AppAddr:�û�������ʼ��ַ.*/
void Iap_Load_App(u32 AppAddr)
{
	if(((*(vu32*)AppAddr)&0x2FFE0000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
	{ 
//#if 1
		printf("start jump to appaddr!\n");
		JumpToApp = (IapFun)*(vu32*)(AppAddr+4); //�û��������ڶ�����Ϊ����ʼ��ַ(�³���λ��ַ)		
		MSR_MSP(*(vu32*)AppAddr); //��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		JumpToApp(); //����PCָ��Ϊ�³���λ�жϺ����ĵ�ַ������ִ��

//#else
//        NVIC_SystemReset();
//#endif
	}
//    else
//    {
//        DebugPrint("add is err\n");
//    }
}

#if 1
//����������ƫ�Ƶ�ַ
//NVIC_VectTab:��ַ
//Offset:ƫ����
//CHECK OK
//091207
void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset)	 
{ 
  	//�������Ϸ���
	assert_param(IS_NVIC_VECTTAB(NVIC_VectTab));
	assert_param(IS_NVIC_OFFSET(Offset));  	 
	SCB->VTOR = NVIC_VectTab|(Offset & (u32)0x1FFFFF80);//����NVIC��������ƫ�ƼĴ���
	//���ڱ�ʶ����������CODE��������RAM��
}

//����NVIC����
//NVIC_Group:NVIC���� 0~4 �ܹ�5�� 
//CHECK OK
//091209
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
	u32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;//ȡ����λ
	temp1<<=8;
	temp=SCB->AIRCR;  //��ȡ��ǰ������
	temp&=0X0000F8FF; //�����ǰ����
	temp|=0X05FA0000; //д��Կ��
	temp|=temp1;	   
	SCB->AIRCR=temp;  //���÷���	    	  				   
}

//����NVIC 
//NVIC_PreemptionPriority:��ռ���ȼ�
//NVIC_SubPriority       :��Ӧ���ȼ�
//NVIC_Channel           :�жϱ��
//NVIC_Group             :�жϷ��� 0~4
//ע�����ȼ����ܳ����趨����ķ�Χ!����������벻���Ĵ���
//�黮��:
//��0:0λ��ռ���ȼ�,4λ��Ӧ���ȼ�
//��1:1λ��ռ���ȼ�,3λ��Ӧ���ȼ�
//��2:2λ��ռ���ȼ�,2λ��Ӧ���ȼ�
//��3:3λ��ռ���ȼ�,1λ��Ӧ���ȼ�
//��4:4λ��ռ���ȼ�,0λ��Ӧ���ȼ�
//NVIC_SubPriority��NVIC_PreemptionPriority��ԭ����,��ֵԽС,Խ����
//CHECK OK
//100329
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	 
{ 
	u32 temp;	
	u8 IPRADDR=NVIC_Channel/4;  //ÿ��ֻ�ܴ�4��,�õ����ַ 
	u8 IPROFFSET=NVIC_Channel%4;//�����ڵ�ƫ��
	IPROFFSET=IPROFFSET*8+4;    //�õ�ƫ�Ƶ�ȷ��λ��
	//MY_NVIC_PriorityGroupConfig(NVIC_Group);//���÷���
	temp=NVIC_PreemptionPriority<<(4-NVIC_Group);	  
	temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
	temp&=0xf;//ȡ����λ

	if(NVIC_Channel<32)NVIC->ISER[0]|=1<<NVIC_Channel;//ʹ���ж�λ(Ҫ����Ļ�,�෴������OK)
	else NVIC->ISER[1]|=1<<(NVIC_Channel-32);    
	//NVIC->IPR[IPRADDR]|=temp<<IPROFFSET;//������Ӧ���ȼ����������ȼ�   	    	  				   
	NVIC->IP[IPRADDR]|=temp<<IPROFFSET;//������Ӧ���ȼ����������ȼ�   	    	  				   
}

//�ⲿ�ж����ú���
//ֻ���GPIOA~G;������PVD,RTC��USB����������
//����:GPIOx:0~6,����GPIOA~G;BITx:��Ҫʹ�ܵ�λ;TRIM:����ģʽ,1,������;2,�Ͻ���;3�������ƽ����
//�ú���һ��ֻ������1��IO��,���IO��,���ε���
//�ú������Զ�������Ӧ�ж�,�Լ�������   
//������...
void Ex_NVIC_Config(u8 GPIOx,u8 BITx,u8 TRIM) 
{
	u8 EXTADDR;
	u8 EXTOFFSET;
	EXTADDR=BITx/4;//�õ��жϼĴ�����ı��
	EXTOFFSET=(BITx%4)*4;

	RCC->APB2ENR|=0x01;//ʹ��io����ʱ��

	AFIO->EXTICR[EXTADDR]&=~(0x000F<<EXTOFFSET);//���ԭ�����ã�����
	AFIO->EXTICR[EXTADDR]|=GPIOx<<EXTOFFSET;//EXTI.BITxӳ�䵽GPIOx.BITx
	
	//�Զ�����
	EXTI->IMR|=1<<BITx;//  ����line BITx�ϵ��ж�
	//EXTI->EMR|=1<<BITx;//������line BITx�ϵ��¼� (������������,��Ӳ�����ǿ��Ե�,��������������ʱ���޷������ж�!)
 	if(TRIM&0x01)EXTI->FTSR|=1<<BITx;//line BITx���¼��½��ش���
	if(TRIM&0x02)EXTI->RTSR|=1<<BITx;//line BITx���¼��������ش���
} 


#endif
