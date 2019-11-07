#include "myiic.h"
#include "delay.h"
#include "debug_usart.h"
#include "tmds181.h"

#define DEVICE_ADDR_ONE		0xBC
#define DEVICE_ARDR_TWO		0xB8

extern unsigned char reg_val[I2C_REG_NUM]; 

/* DeviceAddr: 0xBC, 0XB8 */
static void tmds_WriteOneByte(u8 DeviceAddr, u8 WriteAddr, u8 DataToWrite)
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
u8 tmds_ReadOneByte(u8 DeviceAddr, u8 ReadAddr)
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

void tmds_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);	

    //add by lq
	GPIO_ResetBits(GPIOE, GPIO_Pin_0);
	GPIO_ResetBits(GPIOE, GPIO_Pin_4);
	delay_ms(100);
    //end by lq

	GPIO_SetBits(GPIOE, GPIO_Pin_0);
	GPIO_SetBits(GPIOE, GPIO_Pin_4);
#if 0
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x0c, 0x01);		// 2 dB de-emphasis
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x0a, 0x37);
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x0b, 0x08);	
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x09, 0x0a);		// toggle PD_EN
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x09, 0x06);

	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x0c, 0x01);		// 2 dB de-emphasis
	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x0a, 0x37);	
	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x0b, 0x08);	
	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x09, 0x0a);		// toggle PD_EN
	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x09, 0x06);
#else
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x09, 0x0a);		// toggle PD_EN
	delay_ms(10);
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x09, 0x06);
	delay_ms(10);
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x0b, 0x18);
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x0c, 0x01);		// 2 dB de-emphasis
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x0a, 0x34);		//redriver

	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x09, 0x0a);		// toggle PD_EN
	delay_ms(10);
	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x09, 0x06);
	delay_ms(10);
	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x0b, 0x18);
	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x0c, 0x01);		// 2 dB de-emphasis
	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x0a, 0x34);
#endif
	#if 0		// 手册默认值
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x09, 0x02);		 
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x0a, 0xb1);	
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x0b, 0x00);	
	tmds_WriteOneByte(DEVICE_ADDR_ONE, 0x0c, 0x00);

	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x09, 0x02);		 
	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x0a, 0xb1);	
	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x0b, 0x00);	
	tmds_WriteOneByte(DEVICE_ARDR_TWO, 0x0c, 0x00);
	#endif	
}

void hd3ss215_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_5;	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);	

	GPIO_SetBits(GPIOE, HDMI_OUT1_DXSEL);
	GPIO_SetBits(GPIOE, HDMI_OUT2_DXSEL);	
}

/* the one is RK3399, and another is HI3531A */
void hds3ss215_switch_rk3339(void)
{
    GPIO_ResetBits(GPIOE, HDMI_OUT1_DXSEL);
    GPIO_ResetBits(GPIOE, HDMI_OUT2_DXSEL);    
}

void hds3ss215_switch_pc(void)
{
    GPIO_SetBits(GPIOE, HDMI_OUT1_DXSEL);
    GPIO_SetBits(GPIOE, HDMI_OUT2_DXSEL);
}

/* reg_val[SYS_HDS3SS215]: 1--上电  0--断电 */
static void hds3ss215_switch(void)
{
	if (reg_val[SYS_HDS3SS215_CTL] & BIT7)  // 上电, 3399 直接输出
	{
		//printf("ab\n");
        hds3ss215_switch_rk3339();
	} else {	// 断电, pc信号直接输出
		//printf("ac\n");
        hds3ss215_switch_pc();
	}
}

void hds3ss215_check_switch(void)
{
	if (reg_val[SYS_HDS3SS215_CTL] & BIT0)
	{
		hds3ss215_switch();
		reg_val[SYS_HDS3SS215_CTL] &= ~BIT0;
	}
}

void tmds181_register_ctl(void)
{
    if (BIT0 & reg_val[TMDS_CTL])
    {
        tmds_WriteOneByte(DEVICE_ADDR_ONE, reg_val[TMDS_REG], reg_val[TMDS_REG_VALUE]);
        tmds_WriteOneByte(DEVICE_ARDR_TWO, reg_val[TMDS_REG], reg_val[TMDS_REG_VALUE]);	// 2 dB de-emphasis    
        reg_val[TMDS_CTL] &= ~BIT0;
    }  
    else if (BIT1 & reg_val[TMDS_CTL])
    {
        reg_val[TMDS_REG_VALUE] = tmds_ReadOneByte(DEVICE_ADDR_ONE, reg_val[TMDS_REG]);
        printf("1-%x \n", reg_val[TMDS_REG_VALUE]);
        reg_val[TMDS_REG_VALUE] = tmds_ReadOneByte(DEVICE_ARDR_TWO, reg_val[TMDS_REG]);
        printf("2-%x \n", reg_val[TMDS_REG_VALUE]);
        reg_val[TMDS_CTL] &= ~BIT1;
    }//by lht
    else if (BIT0 & reg_val[TMDS_HDMI1_CTL])
    {
        tmds_WriteOneByte(DEVICE_ADDR_ONE, reg_val[TMDS_HDMI1_REG], reg_val[TMDS_HDMI1_REG_VALUE]);
        reg_val[TMDS_HDMI1_CTL] &= ~BIT0;
    }  
    else if (BIT1 & reg_val[TMDS_HDMI1_CTL])
    {
        reg_val[TMDS_HDMI1_REG_VALUE] = tmds_ReadOneByte(DEVICE_ADDR_ONE, reg_val[TMDS_HDMI1_REG]);
        printf("1-%x \n", reg_val[TMDS_HDMI1_REG_VALUE]);
        reg_val[TMDS_HDMI1_CTL] &= ~BIT1;
    }
    else if (BIT0 & reg_val[TMDS_HDMI2_CTL])
    {
        tmds_WriteOneByte(DEVICE_ARDR_TWO, reg_val[TMDS_HDMI2_REG], reg_val[TMDS_HDMI2_REG_VALUE]);	// 2 dB de-emphasis    
        reg_val[TMDS_HDMI2_CTL] &= ~BIT0;
    }  	
    else if (BIT1 & reg_val[TMDS_HDMI2_CTL])
    {
        reg_val[TMDS_HDMI2_REG_VALUE] = tmds_ReadOneByte(DEVICE_ARDR_TWO, reg_val[TMDS_HDMI2_REG]);
        printf("2-%x \n", reg_val[TMDS_HDMI2_REG_VALUE]);
        reg_val[TMDS_HDMI2_CTL] &= ~BIT1;
    }
}

