///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IT6662_1to4.c>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2015/07/03
//   @fileversion: ITE_SPLITER_1.13
//******************************************/
///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IT6662_1to4.c>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2014/12/17
//   @fileversion: ITE_SPLITER_1.01
//******************************************/
#include "IT6662_Sys.h"

#if (USING_1to8 == iTE_FALSE)
#define _EDID_TAB_
#include "IT6662_DefaultEdid.h"

#include "..\IT6662_Source\Hdmi_Rx.h"
#include "..\IT6662_Source\Hdmi_Tx.h"
#include "..\IT6662_Source\Mhl_Rx.h"
#include "..\IT6662_Source\Mhl_Msc.h"
#include "..\IT6662_Source\Edid.h"
#include "..\IT6662_Source\Cec.h"
#include "..\IT6662_Source\IT6662.h"

#if (defined _MCU_IT6350_)
	#include "../IO_IT6350.h"
#endif

iTE_u16 g_u16Format;

//****************************************************************************
extern iTE_u1 g_u1HdcpEn;
extern iTE_u1 bEnHDCP;
extern iTE_u1 g_bCscChk;
extern EdidInfo*	g_stEdidInfo;
extern iTE_u8 g_u8TxSel, g_u8TxOffset, g_u8TxBank, g_u8TxShift;
extern IT6662_Var	*stIt6662;

EdidInfo	g_stTmpEdidInfo;
EdidInfo	g_stComEdidInfo;
extern EdidInfo	*stCurEdidInfo;
iTE_u8	g_u8EdidMode = EDID_COMPOSE_MIN;
#if (USING_CEC == iTE_TRUE)
iTE_u8	g_u8TxCecInit = 0;
#endif
extern iTE_u8	_CODE	u8ExtVic[4];
#if (defined _MCU_IT6350_)
extern unsigned int ui1msTick;
#endif
//****************************************************************************
extern iTE_u1 IT6662_Init(void);
extern void IT6662_Irq(void);
extern void IT6662_RxHpdTrg(void);
extern void IT6662_CecTxPollingCheck(void);
extern iTE_u8	IT6662_GetReg(iTE_u8	u8I2cAdr, iTE_u16 u16Reg);
//****************************************************************************
void IT6662_EdidLoadDefault(iTE_u8 u8EdidMode)
{
	if(u8EdidMode == EDID_DEFAULT_4K2K){
		iTE_Msg(("IT6662_EdidLoadDefault 4k2k\n\r"));
		Edid_FixTabLoad(u8Edid4K2K, &g_stComEdidInfo);
	}else{
		iTE_Msg(("IT6662_EdidLoadDefault FHD\n\r"));
		Edid_FixTabLoad(u8EdidFHD, &g_stComEdidInfo);
	}
}
//****************************************************************************
iTE_u8 IT6662_EdidCompose_Init(iTE_pu8 pu8TxHpdSta)
{
	iTE_u16	u16Size = sizeof(EdidInfo);
	iTE_pu8	pu8Des = (iTE_pu8)&g_stComEdidInfo;

	*pu8TxHpdSta = stIt6662->u8TxHpdStatus & 0x0F;

	if(*pu8TxHpdSta){
		iTE_u8	u8Temp;
		iTE_pu8	pu8Cur;

		pu8Des = (iTE_pu8)&g_stComEdidInfo;

		for(u8Temp=0; u8Temp < 4; u8Temp++){
			if(*pu8TxHpdSta & (0x01 << u8Temp)){
				*pu8TxHpdSta &= ~(0x01 << u8Temp);
				if(g_stEdidInfo[u8Temp].pstVsdbInfo.u8PaAdr){
#if (USING_COPY_IF_ONE_COMPOSE	== iTE_TRUE)
					if(*pu8TxHpdSta == 0){		// Copy port
						EDID_STAT eStatus;
						HdmiTx_Select(u8Temp);

						stCurEdidInfo = &g_stEdidInfo[u8Temp];
						HdmiTx_Bank(g_u8TxBank);
						eStatus = Edid_ExtGet(u8Temp);
						HdmiTx_Bank(0);
//						HdmiTx_SetOutputMode(eStatus);
						if(EDID_NO_ERR != eStatus){
							IT6662_EdidLoadDefault(EDID_DEFAULT);
						}//EDID_CEA_ERR, EDID_VSDB_ERR, do nothing
						return 0;
					}else
#endif
					{
						pu8Cur = (iTE_pu8)&g_stEdidInfo[u8Temp];
						while(u16Size--){
							*pu8Des++ = *pu8Cur++;
						}
						return	++u8Temp;
					}
				}
			}
		}
	}

	IT6662_EdidLoadDefault(EDID_DEFAULT);
	return 0;
}
//****************************************************************************
void IT6662_EdidCompose_Min(void)
{
	iTE_u8	u8TxHpdSta;
	iTE_u8	u8Temp;

	printf("%s\n\r", __func__);
	u8Temp = IT6662_EdidCompose_Init(&u8TxHpdSta);
	if(u8Temp)
	{
		iTE_u8	u8Edid[0x80];
		iTE_u8	u8Cnt1, u8Cnt2;

		if(u8TxHpdSta)
		{
			for(; u8Temp < 4; u8Temp++)
			{
				if(u8TxHpdSta & (0x01 << u8Temp))
				{
					iTE_u16	u163dSbS, u163dTaB, u163dFP;
					iTE_u8	u8TmpVic, u8FreePos;
					EdidInfo*	pstSrcEdidInfo = &g_stEdidInfo[u8Temp];

					g_stComEdidInfo.u8CeaB3 = g_stComEdidInfo.u8CeaB3 & pstSrcEdidInfo->u8CeaB3 & 0xF0;//CEA B3

					for(u8Cnt1 = 0; u8Cnt1 < 16; u8Cnt1++)//VDB VicList
					{ 																	
						g_stComEdidInfo.pstVdbInfo.u8VicList[u8Cnt1] &= pstSrcEdidInfo->pstVdbInfo.u8VicList[u8Cnt1];
					}

					g_stComEdidInfo.pstVsdbInfo.u83dMulti &= pstSrcEdidInfo->pstVsdbInfo.u83dMulti & 0x80;

					u163dSbS = g_stComEdidInfo.pstVsdbInfo.u163dSbS;
					u163dTaB = g_stComEdidInfo.pstVsdbInfo.u163dTaB;
					u163dFP = g_stComEdidInfo.pstVsdbInfo.u163dFP;
					g_stComEdidInfo.pstVsdbInfo.u163dSbS = 0;
					g_stComEdidInfo.pstVsdbInfo.u163dTaB = 0;
					g_stComEdidInfo.pstVsdbInfo.u163dFP = 0;
					u8FreePos = 0;

					for(u8Cnt1=0; u8Cnt1 < EDID_3D_SUPP_MAX; u8Cnt1++)//VSDB 3D List
					{														
#if 0
						for(u8Cnt2=0; u8Cnt2 < EDID_3D_SUPP_MAX; u8Cnt2++)
						{
#if 1
							if(g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1] == pstSrcEdidInfo->pstVdbInfo.u8Vic3dList[u8Cnt2])
							{
								break;
							}
#else
							u8TmpVic = g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1];
							if(u8TmpVic == pstSrcEdidInfo->pstVdbInfo.u8Vic3dList[u8Cnt2])
							{
								if(u8TmpVicList[u8TmpVic >> 3] & (1 << (u8TmpVic & 0x07)))
								{
									u8Cnt2 = EDID_3D_SUPP_MAX;
								}
								u8TmpVicList[u8TmpVic >> 3] |= (1<< (u8TmpVic & 0x07));
								break;
							}
#endif
							if(pstSrcEdidInfo->pstVdbInfo.u8Vic3dList[u8Cnt2] ==0)
							{
								u8Cnt2 = EDID_3D_SUPP_MAX;
								break;
							}
						}

						if(u8Cnt2 == EDID_3D_SUPP_MAX)
						{
							g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1] = 0x00;
						}
						else
						{
							if(u8FreePos != u8Cnt1)
							{
								g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8FreePos]  = g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1];
								g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1] = 0x00;
							}

							if(g_stComEdidInfo.pstVsdbInfo.u83dMulti)
							{
								if((u163dSbS & (1 << u8Cnt1)) && (pstSrcEdidInfo->pstVsdbInfo.u163dSbS & (1 << u8Cnt2)))
								{
									g_stComEdidInfo.pstVsdbInfo.u163dSbS |= (1 << u8FreePos);
								}
								if((u163dTaB & (1 << u8Cnt1)) && (pstSrcEdidInfo->pstVsdbInfo.u163dTaB & (1 << u8Cnt2)))
								{
									g_stComEdidInfo.pstVsdbInfo.u163dTaB |= (1 << u8FreePos);
								}
								if((u163dFP & (1 << u8Cnt1)) && (pstSrcEdidInfo->pstVsdbInfo.u163dFP & (1 << u8Cnt2)))
								{
									g_stComEdidInfo.pstVsdbInfo.u163dFP |= (1 << u8FreePos);
								}
							}
							u8FreePos++;
						}
#else
						u8TmpVic = g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1];
						if(u8TmpVic)
						{
							iTE_u8	u8Cnt3 = 0;

							for(u8Cnt2 = 0; u8Cnt2 < EDID_3D_SUPP_MAX; u8Cnt2++)
							{
								if(u8TmpVic == pstSrcEdidInfo->pstVdbInfo.u8Vic3dList[u8Cnt2])
								{
									if(u8FreePos != u8Cnt1)
									{
										g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8FreePos]  = u8TmpVic;
										g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1] = 0x00;
									}
									if(g_stComEdidInfo.pstVsdbInfo.u83dMulti){
										if((u163dSbS & (1 << u8Cnt1)) && (pstSrcEdidInfo->pstVsdbInfo.u163dSbS & (1 << u8Cnt2)))
										{
											g_stComEdidInfo.pstVsdbInfo.u163dSbS |= (1 << u8FreePos);
										}
										if((u163dTaB & (1 << u8Cnt1)) && (pstSrcEdidInfo->pstVsdbInfo.u163dTaB & (1 << u8Cnt2)))
										{
											g_stComEdidInfo.pstVsdbInfo.u163dTaB |= (1 << u8FreePos);
										}
										if((u163dFP & (1 << u8Cnt1)) && (pstSrcEdidInfo->pstVsdbInfo.u163dFP & (1 << u8Cnt2)))
										{
											g_stComEdidInfo.pstVsdbInfo.u163dFP |= (1 << u8FreePos);
										}
									}
									u8Cnt3++;
								}
								else if(pstSrcEdidInfo->pstVdbInfo.u8Vic3dList[u8Cnt2] == 0)
								{
									break;
								}
							}
							if(u8Cnt3)
							{
								u8FreePos++;
							}
							else
							{
								g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1] = 0x00;
							}
						}
						else
						{
							break;
						}
#endif
					}

					g_stComEdidInfo.pstVsdbInfo.u8B6 &= pstSrcEdidInfo->pstVsdbInfo.u8B6;//VSDB B6
					if(g_stComEdidInfo.pstVsdbInfo.u8B7 > pstSrcEdidInfo->pstVsdbInfo.u8B7)//VSDB B7
						g_stComEdidInfo.pstVsdbInfo.u8B7 = pstSrcEdidInfo->pstVsdbInfo.u8B7;

					if(0)
					{
						g_stComEdidInfo.pstVsdbInfo.u8ExtVic &= pstSrcEdidInfo->pstVsdbInfo.u8ExtVic;//VSDB 4k2k
					}
					else
					{
						g_stComEdidInfo.pstVsdbInfo.u8ExtVic = 0;
						for(u8Cnt1 = 0; u8Cnt1 < 4; u8Cnt1++)
						{
							if(g_stComEdidInfo.pstVdbInfo.u8VicList[u8ExtVic[u8Cnt1] >> 3] & (1 << (u8ExtVic[u8Cnt1] & 0x07)))
							{
								g_stComEdidInfo.pstVsdbInfo.u8ExtVic |= (1 << (u8Cnt1+1));
							}
						}
					}

					g_stComEdidInfo.u8SpkAlloc &= pstSrcEdidInfo->u8SpkAlloc;//SADB


					u8FreePos = 0;
					for(u8Cnt1 = 0; u8Cnt1 < g_stComEdidInfo.pstAdbInfo.u8AdoCnt; u8Cnt1++)//ADB
					{									
						iTE_Msg(("u8Cnt1[0] = 0x%02X\n\r", g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0]));
						for(u8Cnt2 = 0; u8Cnt2 < pstSrcEdidInfo->pstAdbInfo.u8AdoCnt; u8Cnt2++){
						iTE_Msg(("u8Cnt2[0] = 0x%02X\n\r", pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][0]));
							if((g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0] & 0x78) == (pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][0] & 0x78))
							{
								if((g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0] & 0x07) > (pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][0] & 0x07))
								{
									g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0] = pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][0];
								}
								g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][1] &= pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][1];

								if((g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0] & 0x78) < 0x10)//ADB FMT 0,1
								{								
									g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][2] &= pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][2];
								}
								else if((g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0] & 0x78) < 0x48)//ADB FMT 2~8
								{							
									if(g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][2] > pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][2])
									{
										g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][2] = pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][2];
									}
								}
								else//ADB FMT 9~
								{																						
									g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][2] = 0;
								}

								if(u8FreePos != u8Cnt1)
								{
									g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8FreePos][0] = g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0];
									g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8FreePos][1] = g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][1];
									g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8FreePos][2] = g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][2];
								}
								u8FreePos++;
								break;
							}
						}
					}
					g_stComEdidInfo.pstAdbInfo.u8AdoCnt = u8FreePos;

					u8FreePos = 0;
					for(u8Cnt1 = 0; u8Cnt1 < EDID_NATIVE_MAX; u8Cnt1++)//Native VIC
					{												
						u8TmpVic = g_stComEdidInfo.pstVdbInfo.u8NatList[u8Cnt1];
						if(u8TmpVic)
						{
							for(u8Cnt2 = 0; u8Cnt2 < EDID_NATIVE_MAX; u8Cnt2++)
							{
								if(u8TmpVic == pstSrcEdidInfo->pstVdbInfo.u8NatList[u8Cnt2])
								{
									if(u8FreePos != u8Cnt1)
									{
										g_stComEdidInfo.pstVdbInfo.u8NatList[u8FreePos] = u8TmpVic;
										g_stComEdidInfo.pstVdbInfo.u8NatList[u8Cnt1] = 0;
									}
									u8FreePos++;
									break;
								}
								else if(pstSrcEdidInfo->pstVdbInfo.u8NatList[u8Cnt2] == 0)
								{
									u8Cnt2 = EDID_NATIVE_MAX;
									break;
								}
							}
						}
						else
						{
							break;
						}
						if(u8Cnt2 == EDID_NATIVE_MAX)
						{
							g_stComEdidInfo.pstVdbInfo.u8NatList[u8Cnt1] = 0;
						}
					}
#if 0
					if(g_stComEdidInfo.pstVdbInfo.u8NatList[0] == 0)
					{
						if(g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0])
						{
							g_stComEdidInfo.pstVdbInfo.u8NatList[0] = g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0]|0x80;
						}
					}
#endif
				}
			}

#if 0
			g_stComEdidInfo.pstVdbInfo.u8VicCnt = 0;

			for(u8Cnt1 = 0; u8Cnt1 < 16; u8Cnt1++)
			{
				g_stComEdidInfo.pstVdbInfo.u8VicCnt += Edid_GetBitCnt(g_stComEdidInfo.pstVdbInfo.u8VicList[u8Cnt1]);
			}
#else
			Edid_CalVicCnt(&g_stComEdidInfo.pstVdbInfo);
#endif

			if(g_stComEdidInfo.pstVdbInfo.u8NatList[0] == 0)
			{
				if(g_stComEdidInfo.pstVdbInfo.u8VicCnt)
				{
					if(g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0])
					{
						g_stComEdidInfo.pstVdbInfo.u8NatList[0] = g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0] | 0x80;
					}
					else//assign Max VIC to Native VIC
					{		
						for(u8Cnt1 = 16; u8Cnt1 > 0; )
						{
							u8Cnt1--;
							if(g_stComEdidInfo.pstVdbInfo.u8VicList[u8Cnt1])
							{
								for(u8Cnt2 = 8; u8Cnt2 > 0; )
								{
									u8Cnt2--;
									if(g_stComEdidInfo.pstVdbInfo.u8VicList[u8Cnt1] & (0x01 << u8Cnt2))
									{
										g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0] = u8Cnt1 * 8 + u8Cnt2;
										g_stComEdidInfo.pstVdbInfo.u8NatList[0] = g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0] | 0x80;
										break;
									}
								}
								break;
							}
						}
					}
				}
				else//No VIC
				{		
					g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0] = 0x04;
					g_stComEdidInfo.pstVdbInfo.u8NatList[0] = 0x84;	//720p60
					g_stComEdidInfo.pstVdbInfo.u8VicList[0] = 0x10;
					g_stComEdidInfo.pstVdbInfo.u8VicCnt = 1;
				}
				g_stComEdidInfo.u8CeaB3 |= 0x01;
			}
			else
			{
				for(u8Cnt1 = 0; u8Cnt1 < EDID_NATIVE_MAX; u8Cnt1++)
				{
					if(g_stComEdidInfo.pstVdbInfo.u8NatList[u8Cnt1] == 0)
					{
						break;
					}
				}
				g_stComEdidInfo.u8CeaB3 |= u8Cnt1;
			}
		}
		Edid_InfoShow(&g_stComEdidInfo);

		Edid_RegenBlk1(&g_stComEdidInfo, u8Edid, 1);
		//DTD
		u8TxHpdSta = stIt6662->u8TxHpdStatus;
		for(u8Temp = 0; u8Temp < 4; u8Temp++)
		{
			if((g_stEdidInfo[u8Temp].pstVsdbInfo.u8PaAdr) && ((1 << u8Temp) & u8TxHpdSta))
			{
				iTE_u8	u8EdidDtd[18];
				iTE_u8	u8CheckSum=0;
				iTE_u8	u8TmpDtd;
				iTE_u8	u8NoSupDtd = 0;
				iTE_u8	u8Dtd4k2k = 0;

				HdmiTx_Select(u8Temp);
				HdmiTx_Bank(g_u8TxBank);
				HdmiTx_DdcDisable();
				Edid_ExtGetBlock(0);
				HdmiTx_Bank(0);
				//Blk0 Dtd
				for(u8Cnt1 = 0; u8Cnt1 < 4; u8Cnt1++)
				{
					u8TmpDtd = g_stEdidInfo[u8Temp].u8Blk0Dtd[u8Cnt1];
					if(u8TmpDtd)
					{
						if(g_stComEdidInfo.pstVdbInfo.u8VicList[u8TmpDtd >> 3] & (0x01 << (u8TmpDtd & 0x07)))
						{
							if(u8TmpDtd >=  93)
							{
								u8Dtd4k2k = 1;
							}
						}
						else
						{
							//g_stEdidInfo[u8Temp].u8Blk0Dtd[u8Cnt1] = 0;
							u8NoSupDtd |= (1 << u8Cnt1);
						}
					}
				}

				u8NoSupDtd |= g_stEdidInfo[u8Temp].u8UnSupDtd;

				if(u8NoSupDtd)
				{
					while(u8NoSupDtd)
					{
						for(u8Cnt1 = 0; u8Cnt1 < 4; u8Cnt1++)
						{
							if(u8NoSupDtd & (1 << u8Cnt1))
							{
								u8NoSupDtd &= ~(1 << u8Cnt1);
								Edid_Rb(0x36 + (0x12 * u8Cnt1), 0x12, u8EdidDtd);
								for(u8Cnt2 = 0; u8Cnt2 < 0x12; u8Cnt2++)
								{
									u8CheckSum += u8EdidDtd[u8Cnt2];
								}
								if((u8Dtd4k2k == 0) && ((g_stComEdidInfo.pstVdbInfo.u8VicList[11] & 0xE0) || g_stComEdidInfo.pstVdbInfo.u8VicList[12] || (g_stComEdidInfo.pstVdbInfo.u8VicList[13] & 0x0F)))
								{
									for(u8Cnt2 = 93; u8Cnt2 < 108; u8Cnt2++)
									{
										if(g_stComEdidInfo.pstVdbInfo.u8VicList[u8Cnt2 >> 3] & (0x01 << (u8Cnt2 & 0x07)))
										{
											u8Temp = Edid_GetDtdDes(u8Cnt2, u8EdidDtd);
											u8Dtd4k2k = 1;
											break;
										}
									}
									if(u8Cnt2 >= 108)
									{
										iTE_Msg(("ERROR......Can't found 4k2k support VIC\n\r"));
										u8Temp = Edid_GetDtdDes(g_stComEdidInfo.pstVdbInfo.u8NatList[0] & 0x7F, u8EdidDtd);
									}
								}
								else
								{
									u8Temp = Edid_GetDtdDes(g_stComEdidInfo.pstVdbInfo.u8NatList[0] & 0x7F, u8EdidDtd);
								}
								u8CheckSum -= u8Temp;
								Edid_Wb(0x36 + (0x12 * u8Cnt1), 0x12, u8EdidDtd);
							}
						}
					}
				}
				else
				{
					Edid_Rb(0x36, 0x12, u8EdidDtd);
				}

				u8CheckSum += Edid_R(0x7F);
				Edid_W(0x7F, u8CheckSum);

				//Blk1 Dtd
				u8Cnt2 = u8Edid[2];
				u8Cnt1 = 0;
				u8Temp = 0;
				do
				{
					u8Temp += u8EdidDtd[u8Cnt1];
					u8Edid[u8Cnt2++] = u8EdidDtd[u8Cnt1++];
				}while(u8Cnt1 < 0x12);
				u8Edid[0x7F] -= u8Temp;
				break;
			}
		}

		Edid_FixTabBlkLoad(u8Edid, 1);
	}
}
//****************************************************************************
void IT6662_EdidCompose_J(void)
{
	iTE_u8	u8TxHpdSta;
	iTE_u8	u8Temp;

	printf("%s\n\r", __func__);
	u8Temp = IT6662_EdidCompose_Init(&u8TxHpdSta);
	if(u8Temp){
		iTE_u8	u8Edid[0x80];
		iTE_u8	u8Cnt1, u8Cnt2;

		if(u8TxHpdSta)
		{
			for(; u8Temp < 4; u8Temp++)
			{
				if(u8TxHpdSta & (0x01 << u8Temp))
				{
					iTE_u16	u163dSbS, u163dTaB, u163dFP;
					iTE_u8	u8TmpVic, u8FreePos;
					EdidInfo*	pstSrcEdidInfo = &g_stEdidInfo[u8Temp];

					g_stComEdidInfo.u8CeaB3 = g_stComEdidInfo.u8CeaB3 & pstSrcEdidInfo->u8CeaB3 & 0xF0;	// CEA B3

					for(u8Cnt1 = 0; u8Cnt1 < 16; u8Cnt1++)// VDB VicList
					{ 																	
						g_stComEdidInfo.pstVdbInfo.u8VicList[u8Cnt1] &= pstSrcEdidInfo->pstVdbInfo.u8VicList[u8Cnt1];
					}

					g_stComEdidInfo.pstVsdbInfo.u83dMulti |= pstSrcEdidInfo->pstVsdbInfo.u83dMulti & 0x80;

					u163dSbS = g_stComEdidInfo.pstVsdbInfo.u163dSbS;
					u163dTaB = g_stComEdidInfo.pstVsdbInfo.u163dTaB;
					u163dFP = g_stComEdidInfo.pstVsdbInfo.u163dFP;
					g_stComEdidInfo.pstVsdbInfo.u163dSbS = 0;
					g_stComEdidInfo.pstVsdbInfo.u163dTaB = 0;
					g_stComEdidInfo.pstVsdbInfo.u163dFP = 0;
					u8FreePos = 0;

					for(u8Cnt1=0; u8Cnt1 < EDID_3D_SUPP_MAX; u8Cnt1++)
					{
#if 0					// VSDB 3D List
						for(u8Cnt2=0; u8Cnt2 < EDID_3D_SUPP_MAX; u8Cnt2++)
						{
#if 1
							if(g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1] == pstSrcEdidInfo->pstVdbInfo.u8Vic3dList[u8Cnt2])
							{
								break;
							}
#else
							u8TmpVic = g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1];
							if(u8TmpVic == pstSrcEdidInfo->pstVdbInfo.u8Vic3dList[u8Cnt2])
							{
								if(u8TmpVicList[u8TmpVic >> 3] & (1<< (u8TmpVic & 0x07)))
								{
									u8Cnt2 = EDID_3D_SUPP_MAX;
								}
								u8TmpVicList[u8TmpVic >> 3] |= (1<< (u8TmpVic & 0x07));
								break;
							}
#endif
							if(pstSrcEdidInfo->pstVdbInfo.u8Vic3dList[u8Cnt2] == 0)
							{
								u8Cnt2 = EDID_3D_SUPP_MAX;
								break;
							}

						}


						if(u8Cnt2 == EDID_3D_SUPP_MAX)
						{
							g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1] = 0x00;
						}
						else
						{
							if(u8FreePos != u8Cnt1)
							{
								g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8FreePos]  = g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1];
								g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1] = 0x00;
							}

							if(g_stComEdidInfo.pstVsdbInfo.u83dMulti)
							{
								if((u163dSbS & (1 << u8Cnt1)) && (pstSrcEdidInfo->pstVsdbInfo.u163dSbS & (1 << u8Cnt2)))
								{
									g_stComEdidInfo.pstVsdbInfo.u163dSbS |= (1 << u8FreePos);
								}
								if((u163dTaB & (1 << u8Cnt1)) && (pstSrcEdidInfo->pstVsdbInfo.u163dTaB & (1 << u8Cnt2)))
								{
									g_stComEdidInfo.pstVsdbInfo.u163dTaB |= (1 << u8FreePos);
								}
								if((u163dFP & (1 << u8Cnt1)) && (pstSrcEdidInfo->pstVsdbInfo.u163dFP & (1 << u8Cnt2)))
								{
									g_stComEdidInfo.pstVsdbInfo.u163dFP |= (1 << u8FreePos);
								}
							}
							u8FreePos++;
						}
#else
						u8TmpVic = g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1];
						if(u8TmpVic)
						{
							iTE_u8	u8Cnt3 = 0;

							for(u8Cnt2 = 0; u8Cnt2 < EDID_3D_SUPP_MAX; u8Cnt2++)
							{
								if(u8TmpVic == pstSrcEdidInfo->pstVdbInfo.u8Vic3dList[u8Cnt2])
								{
									if(u8FreePos != u8Cnt1)
									{
										g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8FreePos]  = u8TmpVic;
										g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1] = 0x00;
									}
									if(g_stComEdidInfo.pstVsdbInfo.u83dMulti)
									{
										if((u163dSbS & (1 << u8Cnt1)) && (pstSrcEdidInfo->pstVsdbInfo.u163dSbS & (1 << u8Cnt2)))
										{
											g_stComEdidInfo.pstVsdbInfo.u163dSbS |= (1 << u8FreePos);
										}
										if((u163dTaB & (1 << u8Cnt1)) && (pstSrcEdidInfo->pstVsdbInfo.u163dTaB & (1 << u8Cnt2)))
										{
											g_stComEdidInfo.pstVsdbInfo.u163dTaB |= (1 << u8FreePos);
										}
										if((u163dFP & (1 << u8Cnt1)) && (pstSrcEdidInfo->pstVsdbInfo.u163dFP & (1 << u8Cnt2)))
										{
											g_stComEdidInfo.pstVsdbInfo.u163dFP |= (1 << u8FreePos);
										}
									}
									u8Cnt3++;
								}
								else if(pstSrcEdidInfo->pstVdbInfo.u8Vic3dList[u8Cnt2] == 0)
								{
									break;
								}
							}
							if(u8Cnt3)
							{
								u8FreePos++;
							}
							else
							{
								g_stComEdidInfo.pstVdbInfo.u8Vic3dList[u8Cnt1] = 0x00;
							}
						}
						else
						{
							break;
						}
#endif
					}

					g_stComEdidInfo.pstVsdbInfo.u8B6 &= pstSrcEdidInfo->pstVsdbInfo.u8B6;// VSDB B6
					if(g_stComEdidInfo.pstVsdbInfo.u8B7 > pstSrcEdidInfo->pstVsdbInfo.u8B7)// VSDB B7
						g_stComEdidInfo.pstVsdbInfo.u8B7 = pstSrcEdidInfo->pstVsdbInfo.u8B7;

					if(0)
					{
						g_stComEdidInfo.pstVsdbInfo.u8ExtVic &= pstSrcEdidInfo->pstVsdbInfo.u8ExtVic;// VSDB 4k2k
					}
					else
					{
						g_stComEdidInfo.pstVsdbInfo.u8ExtVic = 0;
						for(u8Cnt1 = 0; u8Cnt1 < 4; u8Cnt1++)
						{
							if(g_stComEdidInfo.pstVdbInfo.u8VicList[u8ExtVic[u8Cnt1] >> 3] & (1 << (u8ExtVic[u8Cnt1] & 0x07)))
							{
								g_stComEdidInfo.pstVsdbInfo.u8ExtVic |= (1 << (u8Cnt1 + 1));
							}
						}
					}

					g_stComEdidInfo.u8SpkAlloc &= pstSrcEdidInfo->u8SpkAlloc;// SADB


					u8FreePos = 0;
					for(u8Cnt1 = 0; u8Cnt1 < g_stComEdidInfo.pstAdbInfo.u8AdoCnt; u8Cnt1++)// ADB
					{									
						iTE_Msg(("u8Cnt1[0] = 0x%02X\n\r", g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0]));
						for(u8Cnt2 = 0; u8Cnt2 < pstSrcEdidInfo->pstAdbInfo.u8AdoCnt; u8Cnt2++){
						iTE_Msg(("u8Cnt2[0] = 0x%02X\n\r", pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][0]));
							if((g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0] & 0x78) == (pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][0] & 0x78))
							{
								if((g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0] & 0x07) > (pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][0] & 0x07))
								{
									g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0] = pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][0];
								}
								g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][1] &= pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][1];

								if((g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0] & 0x78) < 0x10)// ADB FMT 0,1
								{								
									g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][2] &= pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][2];
								}
								else if((g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0] & 0x78) < 0x48)// ADB FMT 2~8
								{							
									if(g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][2] > pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][2])
									{
										g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][2] = pstSrcEdidInfo->pstAdbInfo.u8AdoDes[u8Cnt2][2];
									}
								}
								else// ADB FMT 9~
								{																						
									g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][2] = 0;
								}

								if(u8FreePos != u8Cnt1){
									g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8FreePos][0] = g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][0];
									g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8FreePos][1] = g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][1];
									g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8FreePos][2] = g_stComEdidInfo.pstAdbInfo.u8AdoDes[u8Cnt1][2];
								}
								u8FreePos++;
								break;
							}
						}
					}
					g_stComEdidInfo.pstAdbInfo.u8AdoCnt = u8FreePos;

					u8FreePos = 0;
					for(u8Cnt1 = 0; u8Cnt1 < EDID_NATIVE_MAX; u8Cnt1++)// Native VIC
					{												
						u8TmpVic = g_stComEdidInfo.pstVdbInfo.u8NatList[u8Cnt1];
						if(u8TmpVic)
						{
							for(u8Cnt2 = 0; u8Cnt2 < EDID_NATIVE_MAX; u8Cnt2++)
							{
								if(u8TmpVic == pstSrcEdidInfo->pstVdbInfo.u8NatList[u8Cnt2])
								{
									if(u8FreePos != u8Cnt1)
									{
										g_stComEdidInfo.pstVdbInfo.u8NatList[u8FreePos] = u8TmpVic;
										g_stComEdidInfo.pstVdbInfo.u8NatList[u8Cnt1] = 0;
									}
									u8FreePos++;
									break;
								}
								else if(pstSrcEdidInfo->pstVdbInfo.u8NatList[u8Cnt2] == 0)
								{
									u8Cnt2 = EDID_NATIVE_MAX;
									break;
								}
							}
						}
						else
						{
							break;
						}
						if(u8Cnt2 == EDID_NATIVE_MAX)
						{
							g_stComEdidInfo.pstVdbInfo.u8NatList[u8Cnt1] = 0;
						}
					}

#if 0
					if(g_stComEdidInfo.pstVdbInfo.u8NatList[0] == 0)
					{
						if(g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0])
						{
							g_stComEdidInfo.pstVdbInfo.u8NatList[0] = g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0]|0x80;
						}
					}
#endif
				}
			}
#if 1
			//add extra Vic for JJ request
			for(u8Temp = 0; u8Temp < 4; u8Temp++)
			{
				for(u8Cnt1 = 0; u8Cnt1 < 4; u8Cnt1++)
				{
					g_stComEdidInfo.pstVdbInfo.u8VicList[u8Cnt1] |= 	g_stEdidInfo[u8Temp].pstVdbInfo.u8VicList[u8Cnt1];
				}
				//g_stComEdidInfo.pstVdbInfo.u8VicList[u8Cnt1] |= 	g_stEdidInfo[u8Temp].pstVdbInfo.u8VicList[u8Cnt1] & 0x07;
			}
			//add extra Vic for JJ request
#endif
#if 0
			g_stComEdidInfo.pstVdbInfo.u8VicCnt = 0;
			for(u8Cnt1 = 0; u8Cnt1 < 16; u8Cnt1++){
				g_stComEdidInfo.pstVdbInfo.u8VicCnt += Edid_GetBitCnt(g_stComEdidInfo.pstVdbInfo.u8VicList[u8Cnt1]);
			}
#else
			Edid_CalVicCnt(&g_stComEdidInfo.pstVdbInfo);
#endif
			if(g_stComEdidInfo.pstVdbInfo.u8NatList[0] == 0)
			{
				if(g_stComEdidInfo.pstVdbInfo.u8VicCnt)
				{
					if(g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0])
					{
						g_stComEdidInfo.pstVdbInfo.u8NatList[0] = g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0] | 0x80;
					}
					else//assign Max VIC to Native VIC
					{		
						for(u8Cnt1 = 16; u8Cnt1 > 0; )
						{
							u8Cnt1--;
							if(g_stComEdidInfo.pstVdbInfo.u8VicList[u8Cnt1])
							{
								for(u8Cnt2 = 8; u8Cnt2 > 0; )
								{
									u8Cnt2--;
									if(g_stComEdidInfo.pstVdbInfo.u8VicList[u8Cnt1] & (0x01 << u8Cnt2))
									{
										g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0] = u8Cnt1*8+u8Cnt2;
										g_stComEdidInfo.pstVdbInfo.u8NatList[0] = g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0] | 0x80;
										break;
									}
								}
								break;
							}
						}
					}
				}
				else//No VIC
				{		
					g_stComEdidInfo.pstVdbInfo.u8Vic3dList[0] = 0x04;
					g_stComEdidInfo.pstVdbInfo.u8NatList[0] = 0x84;	// 720p60
					g_stComEdidInfo.pstVdbInfo.u8VicList[0] = 0x10;
					g_stComEdidInfo.pstVdbInfo.u8VicCnt = 1;
				}
				g_stComEdidInfo.u8CeaB3 |= 0x01;
			}
			else
			{
				for(u8Cnt1 = 0; u8Cnt1 < EDID_NATIVE_MAX; u8Cnt1++)
				{
					if(g_stComEdidInfo.pstVdbInfo.u8NatList[u8Cnt1] == 0)
					{
						break;
					}
				}
				g_stComEdidInfo.u8CeaB3 |= u8Cnt1;
			}
		}
		Edid_InfoShow(&g_stComEdidInfo);
		Edid_RegenBlk1(&g_stComEdidInfo, u8Edid, 1);

		// DTD
		u8TxHpdSta = stIt6662->u8TxHpdStatus;
		for(u8Temp = 0; u8Temp < 4; u8Temp++)
		{
			if((g_stEdidInfo[u8Temp].pstVsdbInfo.u8PaAdr)&&((1<<u8Temp) & u8TxHpdSta))
			{
				iTE_u8	u8EdidDtd[18];
				iTE_u8	u8CheckSum=0;
				iTE_u8	u8TmpDtd;
				iTE_u8	u8NoSupDtd = 0;
				iTE_u8	u8Dtd4k2k = 0;

				HdmiTx_Select(u8Temp);
				HdmiTx_Bank(g_u8TxBank);
				HdmiTx_DdcDisable();
				Edid_ExtGetBlock(0);
				HdmiTx_Bank(0);
				// Blk0 Dtd
				for(u8Cnt1 = 0; u8Cnt1 < EDID_BLK0_DTD_MAX; u8Cnt1++)
				{
					u8TmpDtd = g_stEdidInfo[u8Temp].u8Blk0Dtd[u8Cnt1];
					iTE_Msg(("Blk0_Dtd[%d] = %d\n\r", u8Cnt1, u8TmpDtd));
					if(u8TmpDtd){
						if(g_stComEdidInfo.pstVdbInfo.u8VicList[u8TmpDtd >> 3] & (0x01 << (u8TmpDtd & 0x07)))
						{
							if(u8TmpDtd >=  93)
							{
								u8Dtd4k2k = 1;
							}
						}
						else
						{
							//g_stEdidInfo[u8Temp].u8Blk0Dtd[u8Cnt1] = 0;
							u8NoSupDtd |= (1<<u8Cnt1);
						}
					}
				}
				iTE_Msg(("NoSupDtd = 0x%x\n\r", u8NoSupDtd));
				u8NoSupDtd |= g_stEdidInfo[u8Temp].u8UnSupDtd;
				iTE_Msg(("NoSupDtd = 0x%x\n\r", u8NoSupDtd));
				if(u8NoSupDtd)
				{
					while(u8NoSupDtd)
					{
						for(u8Cnt1 = 0; u8Cnt1 < EDID_BLK0_DTD_MAX; u8Cnt1++)
						{
							if(u8NoSupDtd & (1 << u8Cnt1))
							{
								u8NoSupDtd &= ~(1<<u8Cnt1);
								Edid_Rb(0x36 + (0x12 * u8Cnt1), 0x12, u8EdidDtd);
								for(u8Cnt2 = 0; u8Cnt2 < 0x12; u8Cnt2++){
									u8CheckSum += u8EdidDtd[u8Cnt2];
								}
								if((u8Dtd4k2k == 0) && ((g_stComEdidInfo.pstVdbInfo.u8VicList[11] & 0xE0) || g_stComEdidInfo.pstVdbInfo.u8VicList[12] || (g_stComEdidInfo.pstVdbInfo.u8VicList[13] & 0x0F)))
								{
									for(u8Cnt2 = 93; u8Cnt2 < 108; u8Cnt2++)
									{
										if(g_stComEdidInfo.pstVdbInfo.u8VicList[u8Cnt2 >> 3] & (0x01 << (u8Cnt2 & 0x07)))
										{
											u8Temp = Edid_GetDtdDes(u8Cnt2, u8EdidDtd);
											u8Dtd4k2k = 1;
											break;
										}
									}
									if(u8Cnt2 >= 108)
									{
										iTE_Msg(("ERROR......Can't found 4k2k support VIC\n\r"));
										u8Temp = Edid_GetDtdDes(g_stComEdidInfo.pstVdbInfo.u8NatList[0] & 0x7F, u8EdidDtd);
									}
								}
								else
								{
									u8Temp = Edid_GetDtdDes(g_stComEdidInfo.pstVdbInfo.u8NatList[0] & 0x7F, u8EdidDtd);
								}
								u8CheckSum -= u8Temp;
								Edid_Wb(0x36 + (0x12 * u8Cnt1), 0x12, u8EdidDtd);
							}
						}
					}
				}
				else
				{
					Edid_Rb(0x36, 0x12, u8EdidDtd);
				}

				u8CheckSum += Edid_R(0x7F);
				Edid_W(0x7F, u8CheckSum);

				// Blk1 Dtd
				u8Cnt2 = u8Edid[2];
				u8Cnt1 = 0;
				u8Temp = 0;
				do
				{
					u8Temp += u8EdidDtd[u8Cnt1];
					u8Edid[u8Cnt2++] = u8EdidDtd[u8Cnt1++];
				}while(u8Cnt1 < 0x12);
				u8Edid[0x7F] -= u8Temp;
				break;
			}
		}

		Edid_FixTabBlkLoad(u8Edid, 1);
	}
}
//****************************************************************************
iTE_u1 IT6662_EdidGet(iTE_u8 u8EdidMode)
{
//	EDID_STAT eEdidRdSta;

	iTE_Msg(("\n\rIT6662_Edid_Get S %d\n\r", u8EdidMode));
	stCurEdidInfo =&g_stComEdidInfo;
	switch(u8EdidMode)
	{
		case EDID_DEFAULT_FHD:
		case EDID_DEFAULT_4K2K:
			IT6662_EdidLoadDefault(u8EdidMode);
			break;
		case EDID_COPY:
			HdmiTx_Select(EDID_COPY_PORT & 0x03);
			if(stIt6662->u8TxHpdStatus & (0x01 << (EDID_COPY_PORT & 0x03)))
			{
				EDID_STAT eStatus;

				stCurEdidInfo = &g_stEdidInfo[EDID_COPY_PORT & 0x03];
				HdmiTx_Bank(g_u8TxBank);
				eStatus = Edid_ExtGet(EDID_COPY_PORT & 0x03);
				HdmiTx_Bank(0);
//				HdmiTx_SetOutputMode(eStatus);
				if(EDID_NO_ERR != eStatus)
				{
					IT6662_EdidLoadDefault(EDID_DEFAULT);
				}//EDID_CEA_ERR, EDID_VSDB_ERR, do nothing
			}
			else
			{
				IT6662_EdidLoadDefault(EDID_DEFAULT);
			}
			break;
		case EDID_COMPOSE_MIN:
			IT6662_EdidCompose_Min();
			break;
		case EDID_COMPOSE_J:
			IT6662_EdidCompose_J();
			break;
	}
	Edid_InfoShow(stCurEdidInfo);

	iTE_Msg(("IT6662_Edid_Get E %d\n\r\n\r", u8EdidMode));

#if (USING_EDID_CHG_HPD == iTE_TRUE)
{
	iTE_u1	bStatus = iTE_FALSE;
	iTE_pu8	pu8TmpEdidInfo = (iTE_pu8)&g_stTmpEdidInfo;
	iTE_pu8	pu8CurEdidInfo = (iTE_pu8)stCurEdidInfo;
	iTE_u16	u16Temp = sizeof(EdidInfo);
	while(u16Temp--){
		if(pu8TmpEdidInfo[u16Temp] != pu8CurEdidInfo[u16Temp]){
			pu8TmpEdidInfo[u16Temp] = pu8CurEdidInfo[u16Temp];
			bStatus = iTE_TRUE;
		}
	}
	return bStatus;
}
#else
	return iTE_TRUE;
#endif
}
//****************************************************************************

VTiming timing_info[] = {
	{60, 	1280,  	720, 	3300, 	2020, 	750, 	30, 	18, 	24,},
	{65, 	1280,  	720, 	3300, 	2020, 	750, 	30, 	18, 	24,},			
	{61, 	1280, 	720, 	3960, 	2680, 	750, 	30, 	18, 	25,},
	{66, 	1280, 	720, 	3960, 	2680, 	750, 	30, 	18, 	25,},			
	{62, 	1280, 	720,  	3300, 	2020, 	750, 	30, 	22, 	30,},
	{67, 	1280, 	720,  	3300, 	2020, 	750, 	30, 	22, 	30,},	
	{108, 	1280, 	720, 	2500, 	1220, 	750, 	30, 	36, 	48,},
	{109, 	1280, 	720, 	2500, 	1220, 	750, 	30, 	36, 	48,},	
	{32, 	1920, 	1080, 	2750, 	830, 	1125, 	45, 	27, 	24,},
	{72, 	1920, 	1080, 	2750, 	830, 	1125, 	45, 	27, 	24,},			
	{33, 	1920, 	1080, 	2640, 	720, 	1125, 	45, 	28, 	25,},
	{73, 	1920, 	1080, 	2640, 	720, 	1125, 	45, 	28, 	25,},			
	{34, 	1920, 	1080, 	2200, 	280, 	1125, 	45, 	33, 	30,}, 
	{74, 	1920, 	1080, 	2200, 	280, 	1125, 	45, 	33, 	30,}, 			
	{111, 	1920, 	1080,  	2750, 	830, 	1125, 	45, 	54, 	48,},
	{112, 	1920, 	1080,  	2750, 	830, 	1125, 	45, 	54, 	48,},	
	{79, 	1680, 	720,   	3300, 	1620, 	750, 	30, 	18, 	24,},		
	{80, 	1680, 	720,   	3168, 	1488, 	750, 	30, 	18, 	25,},		
	{81, 	1680, 	720,   	2640, 	960, 	750, 	30, 	22, 	30,},		
	{110, 	1680, 	720,   	2750, 	1070, 	750, 	30, 	36, 	48,},		
	{86, 	2560, 	1080,   3750, 	1190, 	1100, 	20, 	26,		24,},		
	{87, 	2560, 	1080,   3200, 	640, 	1125, 	45, 	28, 	25,},		
	{88, 	2560, 	1080,   3520, 	960, 	1125, 	45, 	33, 	30,},		
	{113, 	2560, 	1080,   3750, 	1190, 	1100, 	20, 	52, 	48,},		
	{93, 	3840, 	2160,  	5500, 	1660, 	2250, 	90, 	54, 	24,},
	{103, 	3840, 	2160, 	5500, 	1660, 	2250, 	90, 	54, 	24,},		
	{94, 	3840, 	2160,   5280, 	1440, 	2250, 	90, 	56, 	25,},
	{104, 	3840, 	2160,   5280, 	1440, 	2250, 	90, 	56, 	25,},		
	{95, 	3840, 	2160,   4400, 	560, 	2250, 	90, 	67, 	30,},
	{105, 	3840, 	2160,   4400, 	560, 	2250, 	90, 	67, 	30,},		
	{114, 	3840, 	2160,   5500, 	1660, 	2250, 	90, 	108, 	48,},
	{116, 	3840, 	2160,   5500, 	1660, 	2250, 	90, 	108, 	48,},		
	{98, 	4096, 	2160,   5500, 	1404, 	2250, 	90, 	54, 	24,},	
	{99, 	4096, 	2160,   5280, 	1184, 	2250, 	90, 	56, 	25,},		
	{100, 	4096, 	2160,   4400, 	304, 	2250, 	90, 	67, 	30,},		
	{115, 	4096, 	2160,   5500, 	1404, 	2250,	90, 	108, 	48,},	
	{121, 	5120, 	2160,   7500, 	2380, 	2200, 	40, 	52, 	24,},		
	{122, 	5120, 	2160,   7200, 	2080, 	2200, 	40, 	55, 	25,},		
	{123, 	5120, 	2160,   6000, 	880, 	2200, 	40, 	66, 	30,},	
	{124, 	5120, 	2160,   6250, 	1130, 	2475, 	315, 	118, 	48,},		
	{194, 	7680, 	4320,   11000, 3320, 	4500, 	180, 	108, 	24,},
	{202, 	7680, 	4320,   11000, 3320, 	4500, 	180, 	108, 	24,},		
	{195, 	7680, 	4320,   10800, 3120, 	4400 ,	80, 	110, 	25,},
	{203, 	7680, 	4320,   10800, 3120, 	4400 ,	80, 	110, 	25,},			
	{196, 	7680, 	4320,   9000, 	1320, 	4400, 	80, 	132, 	30,},
	{204, 	7680, 	4320,   9000, 	1320, 	4400, 	80, 	132, 	30,},		
	{197, 	7680, 	4320,   11000, 3320, 	4500, 	180, 	216, 	48,},
	{205, 	7680, 	4320,   11000, 3320, 	4500, 	180, 	216, 	48,},	
	{210, 	10240, 	4320,  	12500, 	2260, 	4950, 	630, 	118, 	24,},		
	{211, 	10240, 	4320,   13500, 	3260, 	4400, 	80,		110, 	25,},		
	{212, 	10240, 	4320,   11000, 	760, 	4500, 	180, 	135, 	30,},		
	{213, 	10240, 	4320,   12500,	2260, 	4950, 	630, 	237, 	48,},
	{17,	720, 	576,   	864, 	144, 	625, 	49, 	31, 	50,},
	{18, 	720, 	576,   	864, 	144, 	625, 	49, 	31, 	50,},		
	{19,	1280, 	720,   	1980, 	700, 	750, 	30, 	37, 	50,},
	{68, 	1280, 	720,   	1980, 	700, 	750, 	30, 	37, 	50,},			
	{20, 	1920, 	1080,  	2640, 	720, 	1125, 	22, 	28, 	50,},	
	{21, 	14402,	576,  	17282, 	288, 	625, 	24, 	15, 	50,},
	{22, 	14402,	576,  	17282, 	288, 	625, 	24, 	15, 	50,},
	{23, 	14402, 	288,   	17282, 	288, 	312, 	24, 	15, 	50,},
	{24, 	14402, 	288,   	17282, 	288, 	312, 	24, 	15, 	50,},
	{23, 	14402, 	288,   	17282, 	288, 	313, 	25, 	15, 	49,},
	{24, 	14402, 	288,   	17282, 	288, 	313, 	25, 	15, 	49,},
	{23, 	14402, 	288,   	17282, 	288, 	314, 	26, 	15, 	49,},
	{24, 	14402,	288,   	17282, 	288, 	314, 	26, 	15, 	49,},
	{25, 	28802, 	576 , 	34562, 	576, 	625, 	24, 	15, 	50,},
	{26, 	28802, 	576 , 	34562, 	576, 	625, 	24, 	15, 	50,},
	{27, 	28802, 	288,   	34562, 	576, 	312, 	24, 	15, 	50,},
	{28, 	28802, 	288,   	34562, 	576, 	312, 	24, 	15, 	50,},
	{27, 	28802, 	288,  	34562, 	576, 	313, 	25, 	15, 	49,},
	{28, 	28802, 	288, 	34562, 	576, 	313, 	25, 	15, 	49,},
	{27, 	28802, 	288,   	34562, 	576, 	314, 	26, 	15, 	49,},
	{28, 	28802, 	288,   	34562, 	576, 	314, 	26, 	15, 	49,},
	{29, 	14402, 	576,   	17282, 	288, 	625, 	49, 	31, 	50,},
	{30, 	14402, 	576,   	17282, 	288, 	625, 	49, 	31, 	50,},
	{31, 	1920, 	1080,  	2640, 	720, 	1125, 	45, 	56, 	50,},
	{75, 	1920, 	1080,  	2640, 	720, 	1125, 	45, 	56, 	50,},
	{37, 	28802, 	576,   	34562, 576, 	625, 	49, 	31, 	50,},
	{38, 	28802, 	576,   	34562, 576, 	625, 	49, 	31, 	50,},
	{39, 	1920, 	1080,  	2304, 	384, 	1250, 	85, 	31, 	50,},
	{82, 	1680, 	720,   	2200,	520, 	750, 	30, 	37, 	50,},
	{89, 	2560, 	1080,   3300, 	740, 	1125, 	45, 	56, 	50,},
	{96, 	3840, 	2160,  	5280, 	1440, 	2250, 	90, 	112, 	50,},
	{106, 	3840, 	2160,  	5280, 	1440, 	2250, 	90, 	112, 	50,},
	{101, 	4096, 	2160,   5280, 	1184, 	2250, 	90, 	112, 	50,},
	{125, 	5120, 	2160,   6600, 	1480, 	2250, 	90, 	112, 	50,},
	{198, 	7680, 	4320,   10800, 3120, 	4400, 	80, 	220, 	50,},
	{206, 	7680, 	4320,   10800, 3120, 	4400, 	80, 	220, 	50,},
	{214, 	10240, 	4320,   13500, 3260, 	4400, 	80, 	220, 	50,},
	
	{1, 	640, 	480,   	800, 	160, 	525, 	45, 	31, 	59,},
	{2, 	720, 	480,   	858, 	138, 	525, 	45, 	31, 	59,},
	{3, 	720, 	480,   	858, 	138, 	525, 	45, 	31, 	59,},
	{4, 	1280,	720,   	1650, 	370, 	750, 	30, 	45, 	60,},
	{69, 	1280, 	720,   	1650, 	370, 	750, 	30, 	45, 	60,},
	{5, 	1920, 	1080,  	2200, 	280, 	1125, 	22, 	33, 	60,},
	{6, 	14402, 	480,  	17162, 	276, 	525, 	22, 	15, 	59,},
	{7, 	14402, 	480,  	17162, 	276, 	525, 	22, 	15, 	59,},
	{8, 	14402, 	240,   	17162, 	276, 	262, 	22, 	15, 	60,},
	{9, 	14402, 	240,   	17162, 	276, 	262, 	22, 	15, 	60,},
	{8, 	14402, 	240,   	17162, 	276, 	263, 	23, 	15, 	59,},
	{9, 	14402, 	240,   	17162, 	276, 	263, 	23, 	15, 	59,},
	{10, 	28802, 	480,  	34322, 	552, 	525, 	22, 	15, 	59,},
	{11, 	28802, 	480,  	34322, 	552, 	525, 	22, 	15, 	59,},
	{12, 	28802, 	240,   	34322, 	552, 	262, 	22, 	15, 	60,},
	{13, 	28802, 	240,   	34322, 	552, 	262, 	22, 	15, 	60,},
	{12, 	28802, 	240,   	34322, 	552, 	263, 	23, 	15, 	59,},
	{13, 	28802, 	240,   	34322, 	552, 	263, 	23, 	15, 	59,},
	{14, 	14402, 	480,   	17162, 	276, 	525, 	45, 	31, 	59,},
	{15, 	14402, 	480,   	17162, 	276, 	525, 	45, 	31, 	59,},
	{16, 	1920, 	1080,   2200, 	280, 	1125, 	45, 	67, 	60,},
	{76, 	1920, 	1080,   2200, 	280, 	1125, 	45, 	67, 	60,},
	{35, 	28802, 	480,   	34322, 552, 	525, 	45, 	31, 	59,},
	{36, 	28802, 	480,   	34322, 552, 	525, 	45, 	31, 	59,},
	{83, 	1680, 	720,   	2200, 	520, 	750, 	30, 	45, 	60,},
	{90, 	2560, 	1080,   3000, 	440, 	1100, 	20, 	66, 	60,},
	{97, 	3840, 	2160,   4400, 	560, 	2250, 	90, 	135, 	60,},
	{107, 	3840, 	2160,   4400, 	560, 	2250, 	90, 	135, 	60,},
	{102, 	4096, 	2160,   4400, 	304, 	2250, 	90, 	135, 	60,},
	{126, 	5120, 	2160,   5500, 	380, 	2250, 	90, 	135, 	60,},
	{199, 	7680, 	4320,   9000, 	1320, 	4400, 	80, 	264, 	60,},
	{207, 	7680, 	4320,   9000, 	1320, 	4400, 	80, 	264, 	60,},
	{215, 	10240, 	4320,   11000, 760, 	4500, 	180, 	270, 	60,},
};

#define VAL 9

iTE_u8 IT6662_Get_VIC(VTiming *pCurVTiming)
{
	int i;
	int FrameRate;
	FrameRate = (int)(pCurVTiming->PCLK * 1000 / pCurVTiming->HTotal / pCurVTiming->VTotal);
	for(i = 0; i < sizeof(timing_info) / sizeof(timing_info[0]); i++)
	{
		if((pCurVTiming->HActive == timing_info[i].HActive) && (pCurVTiming->VActive == timing_info[i].VActive) \
			&& (pCurVTiming->HTotal == timing_info[i].HTotal) \
			&& ((FrameRate >= timing_info[i].VFreq - VAL) && (FrameRate <= timing_info[i].VFreq + VAL)))
		{
			return timing_info[i].Vic;
		}
	}
	printf("match VIC error\n\r");
	return 0x00;
}

int IT6662_ResetHpd_Check()
{
	int i;
	int ret = 0;
	iTE_u8 VIC;
	VTiming CurVTiming;
	HdmiRx_GetVideoTiming(&CurVTiming);
	VIC = IT6662_Get_VIC(&CurVTiming);
	
	printf("\n\r");
	printf("VIC: 0x%02X\n\r", VIC);

	printf("VicList:   ");
	for(i = 0; i < 16; i++)
	{
		printf("%02X ", g_stComEdidInfo.pstVdbInfo.u8VicList[i]);
	}
	printf("\n\r");

	printf("3DVicList: ");
	for(i = 0; i < 16; i++)
	{
		printf("%02X ", g_stComEdidInfo.pstVdbInfo.u8Vic3dList[i]);
	}
	printf("\n\r");

	if(VIC == 0x00)
	{
		return 1;
	}
	for(i = 0; i < 16; i++)
	{
		if((g_stComEdidInfo.pstVdbInfo.u8VicList[i] == VIC) || (g_stComEdidInfo.pstVdbInfo.u8Vic3dList[i] == VIC))
		{
			return 0;
		}
	}
	return 1;
}

void IT6662_SysEdidCheck(void)
{
#if (USING_CEC == iTE_TRUE)
	g_u8TxCecInit |= stIt6662->u8EdidCheck & stIt6662->u8TxHpdStatus;
#endif
	if(stIt6662->u8EdidCheck){
		if(!((g_u8EdidMode == EDID_COPY) && ((EDID_COPY_PORT & 0x03) == HDMI_TX_A)))
		{
			Edid_ExtCheck(HDMI_TX_A);
		}
		if(!((g_u8EdidMode == EDID_COPY) && ((EDID_COPY_PORT & 0x03) == HDMI_TX_B)))
		{
			Edid_ExtCheck(HDMI_TX_B);
		}
		if(!((g_u8EdidMode == EDID_COPY) && ((EDID_COPY_PORT & 0x03) == HDMI_TX_C)))
		{
			Edid_ExtCheck(HDMI_TX_C);
		}
		if(!((g_u8EdidMode == EDID_COPY) && ((EDID_COPY_PORT & 0x03) == HDMI_TX_D)))
		{
			Edid_ExtCheck(HDMI_TX_D);
		}
		if(g_u8EdidMode == EDID_COPY)
		{
			if(stIt6662->u8EdidCheck & (1 << (EDID_COPY_PORT & 0x03)))
			{
				if((EdidUpdateTxChange_Copy) || (stIt6662->u8TxHpdStatus & (1 << (EDID_COPY_PORT & 0x03))))
				{
					if(IT6662_EdidGet(EDID_COPY))
					{
						stIt6662->u8TxTrgRxHpd |= 0x80;
					}
				}
			}
		}
		else if((g_u8EdidMode == EDID_COMPOSE_MIN) || (g_u8EdidMode == EDID_COMPOSE_J))
		{
			if((EdidUpdateTxChange_Compose) || (stIt6662->u8TxHpdStatus & stIt6662->u8EdidCheck))
			{
				if(IT6662_EdidGet(g_u8EdidMode))
				{
					if(IT6662_ResetHpd_Check())
					{ 
						printf("\n\r");
						printf("* * * * * * * * * * * * * * * * * * * * * *\n\r");
						printf("* * * * * * * *reset RX hpd * * * * * * * *\n\r");
						printf("* * * * * * * * * * * * * * * * * * * * * *\n\r");
						printf("\n\r");
						stIt6662->u8TxTrgRxHpd |= 0x80;
					}
					else
					{
						printf("\n\r");
						printf("* * * * * * * * * * * * * * * * * * * * * * * *\n\r");
						printf("* * * * * * * *not reset RX hpd * * * * * * * *\n\r");
						printf("* * * * * * * * * * * * * * * * * * * * * * * *\n\r");
						printf("\n\r");
					}
				}
			}
		}

		if((stIt6662->u8TxTrgRxHpd & 0x80) || (stIt6662->u8TxHpdStatus & stIt6662->u8EdidCheck))
		{
			iTE_u8	u8VsdbB6Adr = (stCurEdidInfo->pstVsdbInfo.u8PaAdr + 2) | 0x80;
			iTE_u8	u8TmpB6 = Edid_ColorDepthCheck();

			if(u8TmpB6 != 0xFF)
			{
				if(Edid_ColorDepthChange(u8TmpB6, u8VsdbB6Adr))
				{
					stIt6662->u8TxTrgRxHpd |= 0x80;
				}
			}
		}

		if(stIt6662->u8TxPortEn)
		{
			if(HdmiTx_R(0x22) & stIt6662->u8TxHpdStatus)
			{
				HdmiTx_Set(0x0A, 0x40, 0x00);
			}
			else
			{
				HdmiTx_Set(0x0A, 0x40, 0x40);			// disable Tx Packet FIFO error INT when all sink are not HDMI
			}
		}
	}

	if(stIt6662->u8EdidCheck || g_bCscChk)
	{
		iTE_u8	u8AVMuteState;

		u8AVMuteState = HdmiTx_R(0x23);
		HdmiRx_CscOutputSet(HdmiTx_CscOutputSet());
		HdmiTx_W(0x23, u8AVMuteState);
		g_bCscChk = 0;

		if((stIt6662->u8EdidCheck & 0x80) == 0x00)
		{
			stIt6662->u8EdidCheck = 0;
		}
	}
}
//****************************************************************************
void IT6662_SysRxHpdTrg(void)
{
	IT6662_RxHpdTrg();

#if 1	// move HdmiTx_PreSpenHdcp from HdmiTx_PortEnable to here
//	iTE_Msg(("u8TxSpenHdcp = %02X\n\r", (stIt6662->u8TxSpenHdcp)));
	if(stIt6662->u8TxSpenHdcp){
//		iTE_Sleep_ms(10);
		iTE_u8	u8Tmp;
		for(u8Tmp = 0; u8Tmp < 0x04; u8Tmp++){
			if(stIt6662->u8TxSpenHdcp & (1<<u8Tmp)){
				HdmiTx_Select(u8Tmp);
				stIt6662->u8TxDoHdcp |= g_u8TxShift;
//				stIt6662->u8HDCPFireCnt[g_u8TxSel]=0;
				HdmiTx_PreSpenHdcp(iTE_TRUE);
			}
		}
	}
#endif
}
//****************************************************************************
void IT6662_SysSetEdidMode(iTE_u8 u8EdidMode)
{
	g_u8EdidMode = u8EdidMode;
}
//****************************************************************************
void IT6662_SysSetLED(void)
{
	iTE_u16	u16Temp, u16LedSet;

#if (USING_IT6661 == iTE_TRUE)
	u16LedSet =  (IT6662_GetReg(HDMI_TX_ADR, 0x19) & 0x0F) >>1;
#else
	u16LedSet =  (IT6662_GetReg(HDMI_TX_ADR, 0x19) & 0x0F);
#endif
	u16Temp = (IT6662_GetReg(HDMI_RX_ADR, 0x0A) & 0x80);
	u16LedSet |= u16Temp << 1;
	if(u16Temp){
		if((IT6662_GetReg(HDMI_RX_ADR, 0x93) & 0x01) && ((IT6662_GetReg(HDMI_RX_ADR, 0xA8) & 0x02) == 0)){
			u16LedSet |= 0x0200;
		}
	}

	iTE_GPO_Set(u16LedSet);
}
//****************************************************************************
void IT6662_SysEdidModeSelect(void)
{
#if (defined _MCU_IT6350_)
	iTE_u8	u8Temp;

	//u8Temp = GPDRG & 0x80;
	//u8Temp |= GPDRC & 0x20;
	u8Temp = 0x20;
	
	switch(u8Temp & 0xA0)
	{
		case KEY_EDID_MODE_FHD:
			IT6662_SysSetEdidMode(EDID_DEFAULT_FHD);
			break;
		case KEY_EDID_MODE_COPY:
			IT6662_SysSetEdidMode(EDID_COPY);
			break;
		case KEY_EDID_MODE_COMPOSE:
			IT6662_SysSetEdidMode(EDID_COMPOSE_J);
			break;
		case KEY_EDID_MODE_UHD:
			IT6662_SysSetEdidMode(EDID_DEFAULT_4K2K);
			break;
	}
#else
	IT6662_SysSetEdidMode(EDID_COMPOSE_MIN);
#endif
}
//****************************************************************************
iTE_u1 IT6662_SysInit(void)
{
	IT6662_SysEdidModeSelect();
	return IT6662_Init();
}
//****************************************************************************
#if (USING_CEC == iTE_TRUE)
void IT6662_CecChangeParameter(void)
{
	extern	stCecVar *g_stCecCur;
	iTE_u8	u8Follower;

	u8Follower = Cec_R(0x10) & 0x0F;
	Cec_W(0x10,  u8Follower | g_stCecCur->u8MyLogAdr << 4);

	switch(Cec_R(0x11)){
		case	eActiveSource:
			Cec_W(0x12, g_stCecCur->u8PaL);
			Cec_W(0x13, g_stCecCur->u8PaH);
			break;
	}
}
//****************************************************************************
void IT6662_TxCecHandler(void)
{
	iTE_u8	u8Temp;
	extern	stCecVar *g_stCecCur;

	Cec_BlockSel(CEC_TX_SEL);
	if(g_stCecCur->bTxCecDone){
		g_stCecCur->u8TxCecInitDone &= stIt6662->u8TxHpdStatus;
		g_stCecCur->u8TxCecFire &= stIt6662->u8TxHpdStatus;
		if(g_stCecCur->u8TxCecFire){
			for(u8Temp = 0; u8Temp < 4; u8Temp++){
				if(g_stCecCur->u8TxCecFire & (0x01 << u8Temp)){
					g_stCecCur->u8TxCecFire &= ~(0x01 << u8Temp);
					Cec_TxSel(u8Temp);
					IT6662_CecChangeParameter();
					Cec_TxFire();
					return;
				}
			}
		}
		if(g_u8TxCecInit){
			for(u8Temp = 0; u8Temp < 4; u8Temp++){
				if(g_u8TxCecInit & (0x01 << u8Temp)){
					g_u8TxCecInit &= ~(0x01 << u8Temp);
					Cec_TxSel(u8Temp);
					//Cec_TxPolling(IT6662_TX_LA);
					Cec_TxPolling(g_stCecCur->u8MyLogAdr);
					return;
				}
			}
		}
		if(Cec_TxCmdPull()){
			g_stCecCur->u8TxCecFire = g_stCecCur->u8TxCecInitDone;
			for(u8Temp = 0; u8Temp < 4; u8Temp++){
				if(g_stCecCur->u8TxCecFire & (0x01 << u8Temp)){
					g_stCecCur->u8TxCecFire &= ~(0x01 << u8Temp);
					Cec_TxSel(u8Temp);
					IT6662_CecChangeParameter();
					Cec_TxFire();
					return;
				}
			}
		}
		Cec_TxSel(IT6662_DEFAULT_CEC);
	}
}
#endif
//****************************************************************************
void IT6662_SysIrq(void)
{
	IT6662_Irq();
	IT6662_SysEdidCheck();
	IT6662_SysRxHpdTrg();
#if (USING_CEC == iTE_TRUE)
	IT6662_TxCecHandler();
#endif
	IT6662_SysSetLED();
#if (defined _MCU_IT6350_)
//	iTE_Msg(("1msTick = %d \n\r", ui1msTick));
#endif
	iTE_ExtIntEn(1);
}
#endif
