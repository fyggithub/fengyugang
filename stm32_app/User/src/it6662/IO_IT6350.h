///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IO_IT6350.h>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2015/07/03
//   @fileversion: ITE_SPLITER_1.13
//******************************************/
///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IO_IT6350.h>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2014/12/17
//   @fileversion: ITE_SPLITER_1.01
//******************************************/
#ifndef _IO_IT6350_H_
#define _IO_IT6350_H_

#include ".\IT6662_Customer\IT6662_config.h"

#define USING_I2C		(1)
#define USING_SMBUS	(1)

#define SMBusA				(3)
#define SMBusB				(4)
#define SMBusC				(5)
#define I2C_SMBusD			(0)
#define I2C_SMBusE			(1)
#define I2C_SMBusF			(2)

iTE_u1 i2c_read_byte( iTE_u8 address, iTE_u8 offset, iTE_u8 byteno, iTE_u8 *p_data, iTE_u8 device );
iTE_u1 i2c_write_byte( iTE_u8 address, iTE_u8 offset, iTE_u8 byteno, iTE_u8 *p_data, iTE_u8 device );
iTE_u1 my_i2c_read_byte( iTE_u8 address, iTE_u8 offset, iTE_u8 byteno, iTE_u8 *p_data, iTE_u8 device );

void ExtInt_Enable(iTE_u1 bEnable);
void mDelay(iTE_u16 Delay_Count);
void mSleep(iTE_u16 Delay_Count);
void GPO_Set(iTE_u16 u16LedSet);
//#endif
//void MCU_Init();
iTE_u1 HOLD_STATUS(void);
iTE_u1 HDCP_REPEATER(void);

extern iTE_u8 CurSysStatus;
//void IT6662_Close();
//void IT6662_Reset();
#endif
