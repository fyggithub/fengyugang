///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IO_IT6350.c>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2015/07/03
//   @fileversion: ITE_SPLITER_1.13
//******************************************/
///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IO_IT6350.c>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2014/12/17
//   @fileversion: ITE_SPLITER_1.01
//******************************************/
#define _IO_IT6350_C_
#include "IO_IT6350.h"
#include "myiic.h"
#include "delay.h"

extern unsigned char reg_val[I2C_REG_NUM]; 


extern 	iTE_u8	u8IntEvent;
extern 	iTE_u8	g_u8CurDev;
void MCU_Init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	printf("\n\r****** MCU_Init ******\n\r\n\r");
	/*  */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;		// 推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);		

    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    delay_ms(50);
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
    delay_ms(50);

    /* HDMI_SPLITER_INT */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	//GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;		// 置成上拉输入
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;		// 浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);	   
    delay_ms(50);

	// 中断线以及中断初始化配置               下降沿触发    
	EXTI_ClearITPendingBit(EXTI_Line0);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
	EXTI_InitStructure.EXTI_Line 		= EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger 	= EXTI_Trigger_Falling;
	//EXTI_InitStructure.EXTI_Trigger 	= EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd 	= ENABLE;
	EXTI_Init(&EXTI_InitStructure);	

	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}


void IT6662_Close()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	CurSysStatus = 3;
	printf("****** IT6662_Close ******\n\r");
	/*  */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;		// 推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);		

    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    delay_ms(50);
}

void IT6662_Reset()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	printf("****** IT6662_Reset ******\n\r");
	/*  */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;		// 推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);		

    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    delay_ms(50);
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
    delay_ms(50);

	CurSysStatus = 0;
}

void it6662_WriteOneByte(u8 DeviceAddr, u8 WriteAddr, u8 DataToWrite)
{
		IIC_Start(IIC_E_BUS0);  
		IIC_SendByte(IIC_E_BUS0, DeviceAddr); //发送器件地址,写数据 
		IIC_WaitAck(IIC_E_BUS0);	   
		IIC_SendByte(IIC_E_BUS0, WriteAddr); //发送地址
		IIC_WaitAck(IIC_E_BUS0); 	 										  		   
		IIC_SendByte(IIC_E_BUS0, DataToWrite); 							   
		IIC_WaitAck(IIC_E_BUS0);  		    	   
		IIC_Stop(IIC_E_BUS0); 
		delay_ms(10);	   
}

/* DeviceAddr: 0xBC, 0XB8 */
//static u8 tmds_ReadOneByte(u8 DeviceAddr, u8 ReadAddr)
u8 it6662_ReadOneByte(u8 DeviceAddr, u8 ReadAddr)
{
    u8 temp = 0;

    IIC_Start(IIC_E_BUS0);
    IIC_SendByte(IIC_E_BUS0, DeviceAddr); 			//发送器件地址,写数据
    IIC_WaitAck(IIC_E_BUS0);
    IIC_SendByte(IIC_E_BUS0, ReadAddr); 			//发送低地址
    IIC_WaitAck(IIC_E_BUS0);
    IIC_Start(IIC_E_BUS0);
    IIC_SendByte(IIC_E_BUS0, (DeviceAddr+0X01));	//进入接收模式
    IIC_WaitAck(IIC_E_BUS0);
    temp = IIC_ReadByte(IIC_E_BUS0, 0);
    IIC_Stop(IIC_E_BUS0);

	return temp;
}

iTE_u1 i2c_read_byte( iTE_u8 address, iTE_u8 offset, iTE_u8 byteno, iTE_u8 *p_data, iTE_u8 device )
{
    u8 i = 0;
    for(i=0; i<byteno; i++)
    {
       p_data[i] = it6662_ReadOneByte(address, offset + i);
    }
    
    return 1;
	
}
iTE_u1 i2c_write_byte( iTE_u8 address, iTE_u8 offset, iTE_u8 byteno, iTE_u8 *p_data, iTE_u8 device )
{
	u8 i = 0;
    for(i=0; i<byteno; i++)
    {
       it6662_WriteOneByte(address, offset + i, p_data[i]);
    }
    return 1;
}

iTE_u1 my_i2c_read_byte( iTE_u8 address, iTE_u8 offset, iTE_u8 byteno, iTE_u8 *p_data, iTE_u8 device )
{
    u8 i = 0;
    for(i=0; i<byteno; i++)
    {
       p_data[i] = it6662_ReadOneByte(address, offset);
    }
    
    return 1;
	
}


#if 0
void ExtInt_Enable(iTE_u1 bEnable)
{
	iTE_u8	ucPaSta;

#if (PCB_VER == 11)
//	iTE_Msg(("ExtInt_Enable\n\r"));
	if(bEnable){
		ucPaSta = GPDRA & 0x43;
//		iTE_Msg(("ucPaSta=0x%x\n\r", ucPaSta));
		if(ucPaSta < 0x40){
			u8IntEvent = 0x07 - ucPaSta;
		}else{
			u8IntEvent = 0x43 - ucPaSta;
//			IER11 |= 1 << (Interrupt_INT91 % 8);
WUC_Enable_WUx_Interrupt(WU83, WUC_Falling);
//	INTC_Enable_INTx(Interrupt_INT91, INT_Trigger_Mode_Set_FallingEdge);
//		iTE_Msg(("En_INT91_IT6662_B\n\r"));
		}
		if((u8IntEvent & 0x01) == 0){
//			IER12 |= 1 << (Interrupt_INT96 % 8);
WUC_Enable_WUx_Interrupt(WU91, WUC_Falling);
//	INTC_Enable_INTx(Interrupt_INT96, INT_Trigger_Mode_Set_FallingEdge);
//		iTE_Msg(("En_INT96_IT6662_C\n\r"));
		}
		if((u8IntEvent & 0x02) == 0){
//			IER12 |= 1 << (Interrupt_INT97 % 8);
WUC_Enable_WUx_Interrupt(WU92, WUC_Falling);
//	INTC_Enable_INTx(Interrupt_INT97, INT_Trigger_Mode_Set_FallingEdge);
//		iTE_Msg(("En_INT97_IT6662_A\n\r"));
		}
//		iTE_Msg(("u8IntEvent=0x%x\n\r", u8IntEvent));
	}else{
		IER11 &= ~(1 << (Interrupt_INT91 % 8));
		IER12 &= ~((1 << (Interrupt_INT96 % 8)) |(1 << (Interrupt_INT97 % 8)));
//		bIntEvent = iTE_FALSE;
	}
#else
	if(bEnable){
		ucPaSta = GPDRI & 0x07;

		u8IntEvent = 0x07 - ucPaSta;
		if((u8IntEvent & 0x04) == 0){
			WUC_Enable_WUx_Interrupt(WU121, WUC_Falling);
//			iTE_Msg(("En_INT91_IT6662_B\n\r"));
		}
		if((u8IntEvent & 0x01) == 0){
			WUC_Enable_WUx_Interrupt(WU119, WUC_Falling);
//			iTE_Msg(("En_INT96_IT6662_C\n\r"));
		}
		if((u8IntEvent & 0x02) == 0){
			WUC_Enable_WUx_Interrupt(WU120, WUC_Falling);
//			iTE_Msg(("En_INT97_IT6662_A\n\r"));
		}
//		iTE_Msg(("ucPaSta=0x%x, u8IntEvent=0x%x\n\r", ucPaSta, u8IntEvent));
	}else{
		IER15 &= ~((1 << (Interrupt_INT124 % 8)) |(1 << (Interrupt_INT125 % 8)) |(1 << (Interrupt_INT126 % 8)));
	}
#endif
}
#endif

void mDelay(iTE_u16 Delay_Count)
{
	delay_ms(Delay_Count);
}
void mSleep(iTE_u16 Delay_Count)
{
	delay_ms(Delay_Count);
}

#if 0
void GPO_Set(iTE_u16 u16LedSet)
{
	iTE_u8 u8Temp;
	volatile unsigned char *ps8LedAdr;
	iTE_u8 u8LedMask;
//	GPIO_Operation_Mode(GPIOF6, OUTPUT, OutputType_Push_Pull);
	for(u8Temp = 0; u8Temp < 16; u8Temp++){
		ps8LedAdr = GpoConf[LedAlloc[u8Temp]].pu8Adr;
		if(ps8LedAdr){
			u8LedMask = GpoConf[LedAlloc[u8Temp]].u8Mask;
			if(u16LedSet & (1 << u8Temp)){
				*ps8LedAdr |= u8LedMask;
			}else{
				*ps8LedAdr &= ~u8LedMask;
			}
		}
	}
}
#endif

#define KSI 0xC0

iTE_u1 HOLD_STATUS(void)
{
//	if(GPDRG & 0x80)
	if(KSI & 0x80)
		return 0;
	else
		return 1;
}

iTE_u1 HDCP_REPEATER(void)
{
	if(KSI & 0x40)
		return 0;
	else
		return 1;
}
//#endif

void it6662_register_ctl(void)
{
    if (BIT0 & reg_val[IT6662_CTL])
    {
        it6662_WriteOneByte(reg_val[IT6662_REG], reg_val[IT6662_OFFSET], reg_val[IT6662_VALUE]);
        printf("write %d,%d,%d\n\r", reg_val[IT6662_REG], reg_val[IT6662_OFFSET], reg_val[IT6662_VALUE]);
        reg_val[IT6662_CTL] &= ~BIT0;
    }  
    else if (BIT1 & reg_val[IT6662_CTL])
    {
        reg_val[IT6662_VALUE] = 0x00;
        reg_val[IT6662_VALUE] = it6662_ReadOneByte(reg_val[IT6662_REG], reg_val[IT6662_OFFSET]);
        printf("read %x \n\r", reg_val[IT6662_VALUE]);
        reg_val[IT6662_CTL] &= ~BIT1;
    }
}


