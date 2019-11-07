///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IT6662_IO.c>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2015/07/03
//   @fileversion: ITE_SPLITER_1.13
//******************************************/
///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IT6662_IO.c>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2014/12/17
//   @fileversion: ITE_SPLITER_1.01
//******************************************/
#include "IT6662_IO.h"
#include "../IT6662_Source/Edid.h"
#include"delay.h"

#define _MCU_IT6350_

#if(USING_1to8 == iTE_FALSE)
	#if (defined  _MCU_8051_)
		#define u8I2CBus		(0)
	#elif (defined WIN32)
		#include "../../Prj-VC/src/io.h"
		#define u8I2CBus		(0)
		iTE_u8	CurSysStatus;
	#elif (defined _MCU_IT6350_)
		#include "../IO_IT6350.h"
		#define u8I2CBus		(I2C_SMBusE)
	#endif
#else
	#if (defined  _MCU_8051_)
		iTE_u8 u8I2CBus = 0;
	#elif (defined WIN32)
		#include "../../Prj-VC/src/io.h"
		iTE_u8 u8I2CBus = 0;
		iTE_u8	CurSysStatus;
	#elif (defined _MCU_IT6350_)
		#include "../IO_IT6350.h"
		iTE_u8 u8I2CBus = I2C_SMBusE;
	#endif
#endif

iTE_u8 g_u8CurDev = IT6662_ROOT;

#if (USING_1to8 == iTE_FALSE)
IT6662_Var stIt6662Var;
EdidInfo g_stEdidInfoAry[4];
IT6662_Var	*stIt6662 = &stIt6662Var;
EdidInfo*	g_stEdidInfo = g_stEdidInfoAry;
void IT6662_DeviceSelect(iTE_u8 u8Device){	}
#else
IT6662_Var stIt6662Var[3];
EdidInfo g_stEdidInfoAry[3][4];
IT6662_Var	*stIt6662;
EdidInfo*	g_stEdidInfo;

iTE_u8 u8I2CAdr = 0;


void IT6662_DeviceSelect(iTE_u8 u8Device)
{
	u8I2CBus = u8Device & 0xF;
	u8I2CAdr = u8Device >> 4;
	if(u8Device == IT6662_A){
		stIt6662 = &stIt6662Var[0];
		g_stEdidInfo = g_stEdidInfoAry[0];
	}else if(u8Device == IT6662_B){
		stIt6662 = &stIt6662Var[1];
		g_stEdidInfo = g_stEdidInfoAry[1];
	}else if(u8Device == IT6662_C){
		stIt6662 = &stIt6662Var[2];
		g_stEdidInfo = g_stEdidInfoAry[2];
	}

	g_u8CurDev = u8Device;
}
#endif


iTE_u16 iTE_BusyWait_ms(iTE_u16 usMsCnt)
{
	delay_ms(usMsCnt);
	return usMsCnt+1;
}
void iTE_Sleep_ms(iTE_u16 usMsCnt)
{
	//mSleep(usMsCnt);
	//delay_ms(usMsCnt-1);
	delay_ms(usMsCnt);
}
void iTE_I2C_ReadBurst(iTE_u8 u8Adr,iTE_u8 u8Offset,iTE_u8 u8Cnt,iTE_u8* pu8Buf)
{
	if(i2c_read_byte( u8Adr + u8I2CAdr, u8Offset, u8Cnt, pu8Buf, u8I2CBus ) == 0){
		extern iTE_u8 CurSysStatus;
		CurSysStatus = 0;
	}
}
void iTE_I2C_WriteBurst(iTE_u8 u8Adr,iTE_u8 u8Offset,iTE_u8 u8Cnt,iTE_u8* pu8Buf)
{
#if 0
	return i2c_write_byte( u8Adr + u8I2CAdr, u8Offset, u8Cnt, pu8Buf, u8I2CBus );
#else
	if(i2c_write_byte( u8Adr + u8I2CAdr, u8Offset, u8Cnt, pu8Buf, u8I2CBus ) == 0){
		extern iTE_u8 CurSysStatus;
		CurSysStatus = 0;
	}
#endif
}

void My_iTE_I2C_ReadBurst(iTE_u8 u8Adr,iTE_u8 u8Offset,iTE_u8 u8Cnt,iTE_u8* pu8Buf)
{
	if(my_i2c_read_byte( u8Adr + u8I2CAdr, u8Offset, u8Cnt, pu8Buf, u8I2CBus ) == 0){
		extern iTE_u8 CurSysStatus;
		CurSysStatus = 0;
	}
}


iTE_u8 iTE_I2C_ReadByte(iTE_u8 u8Adr, iTE_u8 u8Offset)
{
	iTE_u8	u8RdData;
	if(i2c_read_byte( u8Adr + u8I2CAdr, u8Offset, 1, &u8RdData, u8I2CBus ) == 0){
		extern iTE_u8 CurSysStatus;
		CurSysStatus = 0;
	}
	return u8RdData;
}
void iTE_I2C_WriteByte(iTE_u8 u8Adr, iTE_u8 u8Offset, iTE_u8 u8Data)
{
	if(i2c_write_byte( u8Adr + u8I2CAdr, u8Offset, 1, &u8Data, u8I2CBus ) == 0){
		extern iTE_u8 CurSysStatus;
		CurSysStatus = 0;
	}
}
void iTE_I2C_SetByte(iTE_u8 u8Adr,iTE_u8 u8Offset,iTE_u8 u8InvMask, iTE_u8 u8Data )
{
	iTE_u8	u8RdData;
	if(u8InvMask){
		if(~u8InvMask){
			if(i2c_read_byte( u8Adr + u8I2CAdr, u8Offset, 1, &u8RdData, u8I2CBus ) == 0){
				extern iTE_u8 CurSysStatus;
				CurSysStatus = 0;
			}
			u8RdData &= ~u8InvMask;
			u8Data &= u8InvMask;
			u8Data |= u8RdData;
		}
		if(i2c_write_byte( u8Adr + u8I2CAdr, u8Offset, 1, &u8Data, u8I2CBus ) == 0){
			extern iTE_u8 CurSysStatus;
			CurSysStatus = 0;
		}
	}else{
		if(u8Offset){
			// ToDo for other feature
		}else{
			//iTE_Sleep_ms(u8Data);
		}
	}
}
void iTE_ExtIntEn(iTE_u1 bEnable)	//*****
{
//	ExtInt_Enable(bEnable);
}
void iTE_GPO_Set(iTE_u16 u16LedSet)
{
	//GPO_Set(u16LedSet);
}
