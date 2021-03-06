///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <Mhl_RcpUcpHandler.c>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2015/07/03
//   @fileversion: ITE_SPLITER_1.13
//******************************************/
///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <Mhl_RcpUcpHandler.c>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2014/12/17
//   @fileversion: ITE_SPLITER_1.01
//******************************************/
#include "Mhl_RcpUcpHandler.h"
#include "..\IT6662_Source\Hdmi_Tx.h"
#include "..\IT6662_Source\Mhl_Msc.h"
//****************************************************************************
iTE_u1 Msc_RapHandler(RAP_ACTION_CODE eRapCmd)
{
	iTE_Msg(("RX MSC_MSG_RAP => "));

	switch(eRapCmd){
		case RAP_POLL:
				iTE_MRx_Msg(("Poll\n\r"));
			break;
		case RAP_CONTENT_ON:
				iTE_MRx_Msg(("Change to CONTENT_ON state\n\r"));
				HdmiTx_Set(0xC1, 0x0F, 0x00);
			break;
		case RAP_CONTENT_OFF:
				iTE_MRx_Msg(("Change to CONTENT_OFF state\n\r"));
				HdmiTx_Set(0xC1, 0x0F, 0x0F);
			break;
		default:
				iTE_MRx_Msg(("ERROR: Unknown RAP action code 0x%2X !!!\n\r", eRapCmd));
				return iTE_FALSE;
	}
	return iTE_TRUE;
}
//****************************************************************************
iTE_u1 Msc_RcpHandler(iTE_u8 ucRcpKey)
{
	Msc_RcpKeyParse(ucRcpKey);
	switch(ucRcpKey){
		//Todo
		default:
			break;
	}
	return iTE_TRUE;
}
//****************************************************************************
iTE_u1 Msc_UcpHandler(iTE_u8 ucUcpCmd)
{
	iTE_MRx_Msg(("Character Code=%02X ==> ", (int)ucUcpCmd));

	if( ucUcpCmd&0x80 ){
		iTE_MRx_Msg(("ERROR: Unsupported UCP character code !!!\n\r"));
		return iTE_FALSE;
	}else{
		iTE_MRx_Msg(("%c", (int)ucUcpCmd));
		//To do UCP process
		return iTE_TRUE;
	}
}
//****************************************************************************
