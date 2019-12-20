#include "myiic.h"
#include "delay.h"
#include "debug_usart.h"	  
#include "sys.h"
#include "gpio.h"

#define LM75_ADDR_W		0x9E
#define LM75_ADDR_R		0x9F
extern unsigned char reg_val[I2C_REG_NUM]; 
extern unsigned int s_numOf100us;
unsigned int temp_start_time = 0;
int temp_flag = 1;

/** 获取温度 
 * 总共16位 bit0-bit15
 * bit0-bit6: Undefine 
 * D7–D15: Temperature Data. One LSB = 0.5°C. Two's complement format
 *			二进制是补码，负数的话，需要转化
 *          bit15: 表示正负数
 *          bit14-bit8: 表示整数
 *          bit7: 小数，1表示0.5°C.
 * 最大温度范围:  –55°C to 125°C
 */
static void lm75_read_temp(void)
{	
    IIC_Start(IIC_E_BUS0);
    IIC_SendByte(IIC_E_BUS0, LM75_ADDR_R); /* 发送器件地址0X9F,读数据 */
    IIC_WaitAck(IIC_E_BUS0); 
    reg_val[SYS_TEMP_H] = IIC_ReadByte(IIC_E_BUS0, 1);
    reg_val[SYS_TEMP_L] = IIC_ReadByte(IIC_E_BUS0, 0);
    IIC_Stop(IIC_E_BUS0);   
	
/*	IIC_Start(IIC_E_BUS0);
	IIC_SendByte(IIC_E_BUS0, LM75_ADDR_W);
	IIC_WaitAck(IIC_E_BUS0);
	IIC_SendByte(IIC_E_BUS0, 0x0);
	IIC_WaitAck(IIC_E_BUS0);
	
	IIC_Start(IIC_E_BUS0);
	IIC_SendByte(IIC_E_BUS0, LM75_ADDR_R);
	IIC_WaitAck(IIC_E_BUS0); 
	reg_val[SYS_TEMP_H] = IIC_ReadByte(IIC_E_BUS0, 1);
	IIC_Ack(IIC_E_BUS0);
	reg_val[SYS_TEMP_L] = IIC_ReadByte(IIC_E_BUS0, 0);
	IIC_NAck(IIC_E_BUS0);
	IIC_Stop(IIC_E_BUS0);*/
	
	return ;
}

void lm75_check_read_temp(void)
{
//	u8 pBuff[] = "temp : ";
    if (1 == temp_flag)
    {
        temp_start_time = s_numOf100us;
        temp_flag = 0;
    }
      
    if (greater_times(temp_start_time, s_numOf100us, 50000)) // 5s
    {
        lm75_read_temp();
//		Debug_Value_Hex(pBuff,sizeof(pBuff) - 1,reg_val[SYS_TEMP_H]);
        temp_flag = 1;
    }    
}

