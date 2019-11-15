#include "common_init.h"
#include "process.h"
#include "flash.h"
#include "sys.h"
#include "delay.h"
#include "debug_usart.h"
#include "cpld.h"

/** 单板上电初始化 
 *
 * PD6		MAIN_12V_EN
 * PD7		PWR_KEY_RIGHT
 * 当电源按键按下是，PWR_KEY_RIGHT会检测到低电平，然后给MAIN_12V_EN这个脚拉低
 * PWR_KEY_RIGHT不按下时是高电平，按下是低电平
 * 正常上电时，MAIN_12V_EN是高
 */
void cpu_power_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//EXTI_InitTypeDef EXTI_InitStructure;
	//NVIC_InitTypeDef NVIC_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	//GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;		// 置成上拉输入
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;		// 浮空输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;     // 推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);  

    GPIO_SetBits(GPIOA, GPIO_Pin_11);
    GPIO_SetBits(GPIOA, GPIO_Pin_12);
	
	GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;     // 推挽输出
    GPIO_Init(GPIOE, &GPIO_InitStructure);  
	GPIO_SetBits(GPIOE, GPIO_Pin_7);
	
	/* pin_6: am5728 reset, others is PMIC reset pin */
    GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_6 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;		// 推挽输出
	GPIO_Init(GPIOD, &GPIO_InitStructure);	
    
	GPIO_ResetBits(GPIOD, GPIO_Pin_1);
	GPIO_ResetBits(GPIOD, GPIO_Pin_2);
	GPIO_SetBits(GPIOD, GPIO_Pin_3);
	GPIO_SetBits(GPIOD, GPIO_Pin_6);   
	
	
#if 0
	/* power pin */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_7 ;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;		// 置成上拉输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);	

	/* power pin */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_6 | GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;		// 推挽输出
	GPIO_Init(GPIOD, &GPIO_InitStructure);		
	//GPIO_SetBits(GPIOD, GPIO_Pin_6);
	GPIO_ResetBits(GPIOD, GPIO_Pin_6);
	
	GPIO_ResetBits(GPIOD, GPIO_Pin_2);		// AM5728 PMIC PWRON 信号
	GPIO_ResetBits(GPIOD, GPIO_Pin_1);		// AM5728 PMIC 复位	
	GPIO_ResetBits(GPIOD, GPIO_Pin_0);		// AM5728  复位

	GPIO_SetBits(GPIOD, GPIO_Pin_2);		// AM5728 PMIC PWRON 信号
	GPIO_SetBits(GPIOD, GPIO_Pin_1);		// AM5728 PMIC 复位	
	GPIO_SetBits(GPIOD, GPIO_Pin_0);		// AM5728  复位

	// 中断线以及中断初始化配置               下降沿触发    
	EXTI_ClearITPendingBit(EXTI_Line7);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource7);
	EXTI_InitStructure.EXTI_Line 		= EXTI_Line7;
	EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger 	= EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd 	= ENABLE;
	EXTI_Init(&EXTI_InitStructure);	

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
#endif
}

/**
 * 单板检测下降沿，中断 
 */
void EXTI15_10_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
		printf("--%s--%s--%d-- cpu power interrupt \n", __FILE__, __func__, __LINE__);
		GPIO_ResetBits(GPIOD, GPIO_Pin_6);	//  正常上电时，PD6 MAIN_12V_EN是低
		EXTI_ClearITPendingBit(EXTI_Line7);

		GPIO_SetBits(GPIOD, GPIO_Pin_2);		// AM5728 PMIC PWRON 信号
		GPIO_SetBits(GPIOD, GPIO_Pin_1);		// AM5728 PMIC 复位	
		GPIO_SetBits(GPIOD, GPIO_Pin_0);		// AM5728  复位
	}
}

/*******************************************************************
//判断目前是否需要升级，1代表不升级，0代表需要升级，程序继续往下执行
*******************************************************************/
unsigned char reset_vector_valid(void)
{
    u16 data = 0x0;

    STMFLASH_Read(FLASH_ADDR_FLAG + 4, &data, 0x1);
    printf("FLASH_ADDR_APP:data[0]=0x%x\n", data);
    if(UPDATE_FLAG == data) 
    {
        return 0;
    }   
    return 1;
}

int main(void)  
{
	u8 ver = 1;
	int count = 0;
	u16 updateFlag = 0xffff;

	sys_init();
	printf("boot\n");

    if(reset_vector_valid())               			//判断目前是否需要升级，1代表不升级，0代表需要升级，程序继续往下执行
    {
        DebugPrint("load app\n");
        Iap_Load_App(FLASH_ADDR_APP);				//跳转到APP
    }
    Proc_setVer(ver);                               //设置版本号
    Proc_ClrDataCnt();                              
    /* base addr: 0x8000000
     * boot size: 64K 0--0x10000
     * app size : 448K 0x10000-0x80000
     * page size: 2K 
     * 擦除，把所有位写1 */
    bsp_flashEraseStartPage(FLASH_PAGE_OF_APP);      			//擦除从0x08010000以后的所有数据
    STMFLASH_Write(FLASH_ADDR_FLAG+4, &updateFlag, 0x1);        //将存放标志升级的标志位擦除
	
    while(1)
	{
	    count++;
        if (count >= 2000000)
        {
        	printf("boot\n");
        	count = 0;
        }
		Proc_appUpdate();                //接收升级命令
	}
}
