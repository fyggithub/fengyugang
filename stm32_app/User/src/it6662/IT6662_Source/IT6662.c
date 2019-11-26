///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IT6662.c>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2015/07/03
//   @fileversion: ITE_SPLITER_1.13
//******************************************/
///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IT6662.c>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2014/12/17
//   @fileversion: ITE_SPLITER_1.01
//******************************************/
//#include "..\IT6662_Customer\IT6662_config.h"
#include "..\IT6662_Customer\IT6662_IO.h"
#include "..\IT6662_Customer\IT6662_Sys.h"
#include "IT6662.h"

#include "Hdmi_Rx.h"
#include "Hdmi_Tx.h"
#include "Mhl_Rx.h"
#include "Edid.h"
#include "Cec.h"

extern iTE_u8	g_u8EdidMode;
extern EdidInfo*	g_stEdidInfo;
extern iTE_u8 g_u8TxSel, g_u8TxOffset, g_u8TxBank, g_u8TxShift;
extern IT6662_Var	*stIt6662;

extern iTE_u1 IT6662_EdidGet(iTE_u8 u8EdidMode);
extern void EXT_SET(iTE_u1 bSet);
//****************************************************************************
void IT6662_VarInit(IT6662_Var* stCurIt6662Var);
//****************************************************************************
iTE_u1 IT6662_Detect(void)
{
	iTE_u8	ucVenID[2], ucDevID[2], ucRevID;

	HdmiRx_Rb(0x00, 2, ucVenID);
	HdmiRx_Rb(0x02, 2, ucDevID);

	HdmiRx_Rb(0x00, 2, ucVenID);
	HdmiRx_Rb(0x02, 2, ucDevID);

	ucRevID = HdmiRx_R(0x04);

	iTE_Msg(("IT6662 -> 20150702\n\r"));
#if (USING_1to8 == iTE_TRUE)
	iTE_Msg(("1 to 8 splitter\n\r"));
#else
#if (USING_IT6661 == iTE_FALSE)
	iTE_Msg(("1 to 4 splitter\n\r"));
#else
	iTE_Msg(("1 to 2 splitter\n\r"));
#endif
#endif
//	iTE_Msg(("*****TEST*****\n\r"));
	iTE_Msg(("###############################################\n\r"));
	iTE_Msg(("#           			IT6662 Detect           					#\n\r"));
	iTE_Msg(("###############################################\n\r"));

	iTE_Msg(("Current VenID=%02X-%02X\n\r", (int)ucVenID[1], (int)ucVenID[0]));
	iTE_Msg(("Current DevID=%02X-%02X\n\r", (int)ucDevID[1], (int)ucDevID[0]));
	iTE_Msg(("Current RevID=%02X\n\r", (int)ucRevID));

	if( ucVenID[0]!=0x54 || ucVenID[1]!=0x49 || ucDevID[0]!=0x62 || ucDevID[1]!=0x66){// || (ucRevID!=0xB0 && ucRevID!=0xB1)) {	//V1.06_1
		iTE_Msg(("\n\rERROR: Can not find iTE Device !!!\n\r"));
		return iTE_FALSE;
     }

	return iTE_TRUE;
}
//****************************************************************************
iTE_u1 IT6662_Init(void)
{
	iTE_Msg(("\n\rIT6662_Init S\n\r"));
	if(iTE_TRUE == IT6662_Detect()){
		IT6662_VarInit(stIt6662);
		HdmiRx_Init();
		MhlRx_Init();
		HdmiTx_Init();
#if (USING_CEC == iTE_TRUE)
		Cec_BlockSel(CEC_RX_SEL);
		HdmiRx_EnCec();
		Cec_Init();
		Cec_BlockSel(CEC_TX_SEL);
		HdmiTx_EnCec();
		Cec_Init();
#endif

		if(iTE_TRUE == EnIntEDID){
			if(g_u8CurDev == IT6662_ROOT){
				IT6662_EdidGet(	g_u8EdidMode);
				IT6662_DeviceSelect(IT6662_ROOT);
			}
		}

		MhlRx_Set(0x2A, 0x01, 0x01);
		MhlRx_Set(0x0F, 0x10, 0x00);

		
#if 1
		//add by wzl
		//hpd control must set
		HdmiRx_Bank(1);
		//CBUS signals follow registers
		HdmiRx_Set(0xB0, 0x40, 0x40);
		HdmiRx_Bank(0);
		HdmiRx_HpdOut(IT6662_HPD_H);
		HdmiTx_Set(0x0D, 0x0F, 0x0C);
#if 0		
		HdmiTx_Select(HDMI_TX_A);
		HdmiTx_PortEnable(1);

		HdmiTx_Select(HDMI_TX_B);
		HdmiTx_PortEnable(1);

		HdmiTx_Select(HDMI_TX_C);
		HdmiTx_PortEnable(1);

		HdmiTx_Select(HDMI_TX_D);
		HdmiTx_PortEnable(1);
#endif
#endif
		printf("****** %s: %s: %d 0x22: 0x%02x ******\n\r", __FILE__, __func__, __LINE__, HdmiTx_R(0x22));
		iTE_Msg(("IT6662_Init E\n\r"));
		return iTE_TRUE;
	}

	return iTE_FALSE;
}
//****************************************************************************
void IT6662_Irq(void)
{
	iTE_u8	u8IntSta;

//	iTE_Msg(("IT6662_Irq S\n\r"));

	u8IntSta = HdmiRx_R(0xE0);
#if (A0_WorkAround == iTE_TRUE)
	u8IntSta = 0x70;
#endif
	if(u8IntSta & 0x40) {
		Mhl_CBusIrq();
		Mhl_MscIrq();
	}
	if(u8IntSta & 0x20) {
		HdmiRx_Irq();
	}

	if(u8IntSta & 0x1F) {	//
		HdmiTx_Irq();
	}

#if (USING_CEC == iTE_TRUE)
	u8IntSta = HdmiRx_R(0xE1);
	if(u8IntSta & 0x02){
		Cec_BlockSel(CEC_TX_SEL);
		Cec_Irq();
	}

	if(u8IntSta & 0x01){
		Cec_BlockSel(CEC_RX_SEL);
		Cec_Irq();
	}

#endif
}
//****************************************************************************
void IT6662_VarInit(IT6662_Var* stCurIt6662Var)
{
	iTE_u16	u16Temp = sizeof(IT6662_Var);
	iTE_pu8	pu8Tmp = (iTE_u8 *)stCurIt6662Var;

	while(u16Temp--){
		*pu8Tmp++ = 0;
	};
	u16Temp = sizeof(EdidInfo)*4;
	pu8Tmp = (iTE_u8 *)g_stEdidInfo;
	while(u16Temp--){
		*pu8Tmp++ = 0;
	};
}
//****************************************************************************
void IT6662_ClrHpdCnt(void)
{
	stIt6662->u8TxTrgRxHpd = 0;
	stIt6662->u8TxHpdCnt[0]  = 0;
	stIt6662->u8TxHpdCnt[1]  = 0;
	stIt6662->u8TxHpdCnt[2]  = 0;
	stIt6662->u8TxHpdCnt[3]  = 0;
}
//****************************************************************************
void IT6662_RxHpdTrg(void)
{
#if (USING_CASCADE == iTE_TRUE)
	if(stIt6662->u8TxTrgRxHpd)
	{
		iTE_Msg(("\n\r%s %d\n\r", __func__, __LINE__));
		iTE_Msg(("%d TxTrgRxHpd = 0x%02X\n\r", stIt6662->bRxHpdStatus, stIt6662->u8TxTrgRxHpd));
		if(stIt6662->bRxHpdStatus)
		{
#if (USING_REPEATER == iTE_TRUE)
			HdmiRx_TrgHpd(20);
#else
			if(stIt6662->u8TxTrgRxHpd & 0x80)
			{
				HdmiRx_TrgHpd(20);
			}
#endif
		}
		else 
		{
			HdmiRx_TrgHpd(1);
		}

		IT6662_ClrHpdCnt();
	}
	else
	{
		if(stIt6662->bRxHpdStatus && ((stIt6662->u8TxHpdStatus & TX_HPD_MASK) == 0))
		{
			iTE_Msg(("\n\r%s %d\n\r", __func__, __LINE__));
			iTE_Msg(("%d TxTrgRxHpd = 0x%02X\n\r", stIt6662->bRxHpdStatus, stIt6662->u8TxHpdStatus));
			HdmiRx_HpdClr(0);
		}
	}
#endif
}
//****************************************************************************

//****************************************************************************
iTE_u8	IT6662_GetReg(iTE_u8	u8I2cAdr, iTE_u16 u16Reg)
{
	iTE_u8	u8RegValue;
	iTE_u8	u8Temp = u16Reg >> 8;
	if(u8Temp){
		iTE_I2C_WriteBurst(u8I2cAdr, 0x0F, 1, &u8Temp);
	}
	iTE_I2C_ReadBurst(u8I2cAdr, u16Reg & 0xFF, 1, &u8RegValue);
	if(u8Temp){
		u8Temp = 0;
		iTE_I2C_WriteBurst(u8I2cAdr, 0x0F, 1, &u8Temp);
	}
	return u8RegValue;
}
//****************************************************************************
void	IT6662_SetReg(iTE_u8	u8I2cAdr, iTE_u16 u16Reg, iTE_u8 u8Data)
{
	iTE_u8	u8Temp = u16Reg >> 8;
	if(u8Temp){
		iTE_I2C_WriteBurst(u8I2cAdr, 0x0F, 1, &u8Temp);
	}
	iTE_I2C_WriteBurst(u8I2cAdr, u16Reg & 0xFF, 1, &u8Data);
	if(u8Temp){
		u8Temp = 0;
		iTE_I2C_WriteBurst(u8I2cAdr, 0x0F, 1, &u8Temp);
	}
}
