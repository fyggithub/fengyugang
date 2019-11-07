///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IT6662_IT6350_Main.c>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2015/07/03
//   @fileversion: ITE_SPLITER_1.13
//******************************************/
///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IT6662_IT6350_Main.c>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2014/12/17
//   @fileversion: ITE_SPLITER_1.01
//******************************************/
#include <stdio.h>
#include ".\IT6662_Customer\IT6662_Sys.h"
#if (defined _MCU_IT6350_)
#include "IO_IT6350.h"
#endif

//extern void ExtInt_Enable(iTE_u1 bEnable);
//extern UINT8 u8IntEvent;

#define HDMI_RX_ADR		(0x90)
#define HDMI_TX_ADR		(0x98)
#define MHL_RX_ADR			(0xE0)
#define EDID_ADR			(0xA8)
//****************************************************************************
iTE_u8	CurSysStatus = 0;
//****************************************************************************
iTE_u1 HoldSystem(void)
{
	static iTE_u8	u8DumpReg = 0;
	if(HOLD_STATUS())
	{
		if(u8DumpReg)
		{
			iTE_u8	u8RegData[0x10], i;
			printf("HDMI_Rx Bank0:\n\r");
			u8RegData[0] = 0x00;
			i2c_write_byte(HDMI_RX_ADR, 0x0F, 1, u8RegData, 1);
			for(i = 0; i < 0x10; i++){
				i2c_read_byte( HDMI_RX_ADR, i << 4, 0x10, u8RegData, 1);
				printf("0x%02X: %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X\n\r",
					i<<4, u8RegData[0], u8RegData[1], u8RegData[2], u8RegData[3], u8RegData[4], u8RegData[5], u8RegData[6], u8RegData[7], u8RegData[8], u8RegData[9], u8RegData[10], u8RegData[11], u8RegData[12], u8RegData[13], u8RegData[14], u8RegData[15] );
			}
			printf("HDMI_Rx Bank1:\n\r");
			u8RegData[0] = 0x01;
			i2c_write_byte(HDMI_RX_ADR, 0x0F, 1, u8RegData, 1);
			for(i = 0; i < 0x10; i++)
			{
				i2c_read_byte( HDMI_RX_ADR, i << 4, 0x10, u8RegData, 1);
				printf("0x%02X: %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X\n\r",
					i<<4, u8RegData[0], u8RegData[1], u8RegData[2], u8RegData[3], u8RegData[4], u8RegData[5], u8RegData[6], u8RegData[7], u8RegData[8], u8RegData[9], u8RegData[10], u8RegData[11], u8RegData[12], u8RegData[13], u8RegData[14], u8RegData[15] );
			}
			printf("HDMI_Rx Bank2:\n\r");
			u8RegData[0] = 0x02;
			i2c_write_byte(HDMI_RX_ADR, 0x0F, 1, u8RegData, 1);
			for(i = 0; i < 0x10; i++)
			{
				i2c_read_byte( HDMI_RX_ADR, i << 4, 0x10, u8RegData, 1);
				printf("0x%02X: %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X\n\r",
					i<<4, u8RegData[0], u8RegData[1], u8RegData[2], u8RegData[3], u8RegData[4], u8RegData[5], u8RegData[6], u8RegData[7], u8RegData[8], u8RegData[9], u8RegData[10], u8RegData[11], u8RegData[12], u8RegData[13], u8RegData[14], u8RegData[15] );
			}
			u8RegData[0] = 0x00;
			i2c_write_byte(HDMI_RX_ADR, 0x0F, 1, u8RegData, 1);

			printf("MHL_Rx Bank:\n\r");
			for(i = 0; i < 0x10; i++)
			{
				i2c_read_byte( MHL_RX_ADR, i << 4, 0x10, u8RegData, 1);
				printf("0x%02X: %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X\n\r",
					i<<4, u8RegData[0], u8RegData[1], u8RegData[2], u8RegData[3], u8RegData[4], u8RegData[5], u8RegData[6], u8RegData[7], u8RegData[8], u8RegData[9], u8RegData[10], u8RegData[11], u8RegData[12], u8RegData[13], u8RegData[14], u8RegData[15] );
			}

			printf("HDMI_Tx Bank0:\n\r");
			u8RegData[0] = 0x00;
			i2c_write_byte(HDMI_TX_ADR, 0x0F, 1, u8RegData, 1);
			for(i = 0; i < 0x10; i++)
			{
				i2c_read_byte( HDMI_TX_ADR, i << 4, 0x10, u8RegData, 1);
				printf("0x%02X: %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X\n\r",
					i<<4, u8RegData[0], u8RegData[1], u8RegData[2], u8RegData[3], u8RegData[4], u8RegData[5], u8RegData[6], u8RegData[7], u8RegData[8], u8RegData[9], u8RegData[10], u8RegData[11], u8RegData[12], u8RegData[13], u8RegData[14], u8RegData[15] );
			}
			printf("HDMI_Tx Bank1:\n\r");
			u8RegData[0] = 0x01;
			i2c_write_byte(HDMI_TX_ADR, 0x0F, 1, u8RegData, 1);
			for(i = 0; i < 0x10; i++)
			{
				i2c_read_byte( HDMI_TX_ADR, i << 4, 0x10, u8RegData, 1);
				printf("0x%02X: %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X\n\r",
					i<<4, u8RegData[0], u8RegData[1], u8RegData[2], u8RegData[3], u8RegData[4], u8RegData[5], u8RegData[6], u8RegData[7], u8RegData[8], u8RegData[9], u8RegData[10], u8RegData[11], u8RegData[12], u8RegData[13], u8RegData[14], u8RegData[15] );
			}
			u8RegData[0] = 0x00;
			i2c_write_byte(HDMI_TX_ADR, 0x0F, 1, u8RegData, 1);

			printf("EDID Bank:\n\r");
			for(i = 0; i < 0x10; i++)
			{
				i2c_read_byte( EDID_ADR, i << 4, 0x10, u8RegData, 1);
				printf("0x%02X: %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X\n\r",
					i<<4, u8RegData[0], u8RegData[1], u8RegData[2], u8RegData[3], u8RegData[4], u8RegData[5], u8RegData[6], u8RegData[7], u8RegData[8], u8RegData[9], u8RegData[10], u8RegData[11], u8RegData[12], u8RegData[13], u8RegData[14], u8RegData[15] );
			}
			u8DumpReg = 0;
		}
     	printf("Hold\\\r");
     	printf("Hold-\r");
     	printf("Hold/\r");
     	printf("Hold|\r");
		return iTE_TRUE;
	}
	else
	{
		u8DumpReg = 1;
		return iTE_FALSE;
	}
}

extern unsigned int s_numOf100us;
int it6662_first_flag = 0;
unsigned int it6662_first_time = 0;
int it6662_check_delay = 1;

//****************************************************************************
void IT6662_Main(void)
{
	//第一次调用
	if(it6662_first_flag == 0)
	{
		it6662_first_time = s_numOf100us;
		it6662_first_flag = 1;
		return;
	}
	else
	{
		if(it6662_check_delay == 1)
		{
			if (greater_times(it6662_first_time, s_numOf100us, 50000))
	        {
				it6662_check_delay = 0;
			}
			else
			{
				return;
			}
		}
	}
	//关闭
	if(CurSysStatus == 3)
	{
		return;
	}
	if(CurSysStatus == 0){
		MCU_Init();
		CurSysStatus = 1;
	}else if(!HoldSystem()){
		switch(CurSysStatus) {
			case 0x1:
				if(IT6662_SysInit()){
					CurSysStatus = 2;
				}
				break;
			case 0x2:
				IT6662_SysIrq();
				break;
			default:
				CurSysStatus=0;
				break;
		}
	}
}
