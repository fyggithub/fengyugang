#include "sys.h"
#include "debug_usart.h"

typedef  void (*IapFun)(void);				//定义一个函数类型的参数
IapFun JumpToApp; 

/*
*THUMB指令不支持汇编内联
*采用如下方法实现执行汇编指令WFI  
*/
void WFI_SET(void)
{
	__ASM volatile("wfi");		  
}

/*关闭所有中断*/
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}

/*开启所有中断*/
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}



/*
*设置栈顶地址
*addr:栈顶地址
*/

__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}


/*跳转到应用程序 AppAddr:用户代码起始地址.*/
void Iap_Load_App(u32 AppAddr)
{
//	vu32 jumpaddress = 0; 
	if(((*(vu32*)AppAddr) & 0x2FFE0000) == 0x20000000)	//检查栈顶地址是否合法.
	{ 
        DebugPrint("addr is ok!\n");
		JumpToApp = (IapFun)*(vu32*)(AppAddr+4); //用户代码区第二个字为程序开始地址(新程序复位地址)		
		MSR_MSP(*(vu32*)AppAddr);		 //初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		JumpToApp();			//设置PC指针为新程序复位中断函数的地址，往下执行
//		jumpaddress = *(__IO u32*)(AppAddr+4);
//		JumpToApp = (IapFun)jumpaddress;
//		__set_MSP(*(__IO u32*)AppAddr);
//		JumpToApp();			//设置PC指针为新程序复位中断函数的地址，往下执行
	}
}


