#include "sys.h"
#include "debug_usart.h"

typedef  void (*IapFun)(void);				//����һ���������͵Ĳ���
IapFun JumpToApp; 

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
    MSR MSP, r0 			//set Main Stack value
    BX r14
}


/*��ת��Ӧ�ó��� AppAddr:�û�������ʼ��ַ.*/
void Iap_Load_App(u32 AppAddr)
{
//	vu32 jumpaddress = 0; 
	if(((*(vu32*)AppAddr) & 0x2FFE0000) == 0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
	{ 
        DebugPrint("addr is ok!\n");
		JumpToApp = (IapFun)*(vu32*)(AppAddr+4); //�û��������ڶ�����Ϊ����ʼ��ַ(�³���λ��ַ)		
		MSR_MSP(*(vu32*)AppAddr);		 //��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		JumpToApp();			//����PCָ��Ϊ�³���λ�жϺ����ĵ�ַ������ִ��
//		jumpaddress = *(__IO u32*)(AppAddr+4);
//		JumpToApp = (IapFun)jumpaddress;
//		__set_MSP(*(__IO u32*)AppAddr);
//		JumpToApp();			//����PCָ��Ϊ�³���λ�жϺ����ĵ�ַ������ִ��
	}
}


