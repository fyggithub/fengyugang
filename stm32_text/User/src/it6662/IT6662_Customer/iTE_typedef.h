///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE_typedef.h>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2015/07/03
//   @fileversion: ITE_SPLITER_1.13
//******************************************/
///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE_typedef.h>
//   @author Hanming.Cheng@ite.com.tw
//   @date   2014/12/17
//   @fileversion: ITE_SPLITER_1.01
//******************************************/
#ifndef _ITE_TYPEDEF_H_
#define _ITE_TYPEDEF_H_

//////////////////////////////////////////////////
// data type
//////////////////////////////////////////////////
#define iTE_FALSE	0
#define iTE_TRUE		1

#ifdef _MCU_8051_
	typedef bit iTE_u1 ;
	#define _CODE code
#else
#ifdef WIN32
	typedef int iTE_u1 ;
	#define _CODE
#else
	typedef int iTE_u1 ;
//	#define _CODE
	#define _CODE __attribute__ ((section ("._OEM_BU1_RODATA ")))
#endif // _MCU_8051_
#endif

typedef enum _iTE_STATUS {
    iTE_SUCCESS = 0,
    iTE_FAIL
//    SYS_RESERVED
} iTE_STATUS;

#ifndef NULL
	#define NULL ((void *) 0)
#endif

typedef char iTE_s8, *iTE_ps8;
typedef unsigned char iTE_u8, *iTE_pu8;
typedef short iTE_s16, *iTE_ps16;
typedef unsigned short iTE_u16, *iTE_pu16;


#ifndef _MCU_8051_				// IT6350
typedef unsigned int iTE_u32, *iTE_pu32;
typedef int iTE_s32, *iTE_ps32;
#else
typedef unsigned long iTE_u32, *iTE_pu32;
typedef long iTE_s32, *iTE_ps32;
#endif

#define CD8BIT			4
#define CD10BIT	 		5
#define CD12BIT			6
#define CD16BIT			7

#define OUT8B           		0
#define OUT10B          		1
#define OUT12B          		2

#define RGB444			0
#define YCbCr422			1
#define YCbCr444			2

#define RGB444_SDR		0
#define YUV444_SDR		1
#define RGB444_DDR		2
#define YUV444_DDR		3
#define YUV422_EMB_SYNC_SDR	4
#define YUV422_EMB_SYNC_DDR	5
#define YUV422_SEP_SYNC_SDR	6
#define YUV422_SEP_SYNC_DDR	7
#define CCIR656_EMB_SYNC_SDR	8
#define CCIR656_EMB_SYNC_DDR	9
#define CCIR656_SEP_SYNC_SDR	10
#define CCIR656_SEP_SYNC_DDR	11
#define RGB444_HALF_BUS		12
#define YUV444_HALF_BUS		13
#define BTA1004_SDR				14
#define BTA1004_DDR				15


typedef struct _IT6662_Var{
//HDMI_TX
	iTE_u8 u8HDCPFireCnt[4];				// Counter for HDCP reAuthentication
	iTE_u32 u8HDCPRiChkCnt[4];			// Counter for HDCP Ri check.
	iTE_u8 u8TxHpdChkTimer[4];			// Counter for Check the Low status period of HPD
	iTE_u8 u8TxHpdCnt[4];				//
	iTE_u8 u8EdidCheck;					// The flag for HPD status changed, EDID may need to check
	iTE_u8 u8TxPortEn;					// The flag for record Tx port enable status, HPD == 1 && RxSen == 1
	iTE_u8 u8TxHpdStatus;				// The flag for record Tx HPD status
	iTE_u16 u16DevCnt;					// HDCP device count for HDCP repeater use
	iTE_u8 u8MaxDepth;					// The Max HDCP device depth
	iTE_u8 u8TxDoHdcp;					// Tx doing HDCP
	iTE_u8 u8TxTrgRxHpd;				// [3:0] The flag for record Tx HPD status changed from Low to High, [7] EDID changed.
	iTE_u8 u8TxSyncDetCnt[4];			// Counter for HDCP Sync Detect.
	iTE_u8 u8TxSpenHdcp;

	iTE_u8 u8TxRptRdyCnt[4];				// Counter for Check HDCP Ready bit in repeater mode.

//HDMI_RX
	iTE_u1 bForceCsc;					// Force Change to RGB444 mode
	iTE_u1 bRxTrgTxHdcp;				// The flag for record Rx trigger
	iTE_u1 bRxHpdStatus;					// The flag for record Rx HPD status
	iTE_u8 u8EccErrCnt;					// ECC error counter
	iTE_u8 u8EqFailCnt;					// EQ fail counter
	iTE_u8 u8DeskewFailCnt;				// DeSkew Fail counter
	iTE_u8 u8HpdTimerCnt;				//
//	iTE_u8 u8OutClr;
//MHL_RX
	iTE_u32 u32RefClk;					// IT6662 reference clock
	iTE_u16 u16WakeFailCnt;				// CBus wakeup fail counter
	iTE_u8 u8DisvFailCnt;					// CBus descover fail counter
	iTE_u8 u8WakeFailSleepCnt;			// Sleep counter for wakeup fail interrupt
	iTE_u1 bCbusDiscov;					// CBus Discover down.
	//iTE_u8 u8EccErrSleepCnt;
	//iTE_u8 u8ClkVaildSleepCnt;
	//iTE_u8 u8ClkRdySleepCnt;
	//iTE_u8 u8ClkStabSleepCnt;
//CEC_TX
}IT6662_Var;


#define GP00				(0)
#define GPB3				(1)
#define GPB4				(2)
#define GPF6				(3)
#define GPF7				(4)
#define GPA0				(5)
#define GPA1				(6)
#define GPG3				(7)
#define GPG4				(8)
#define GPG6				(9)
#define GPA6				(10)
#define GPI0				(11)
#define GPI1				(12)
#define GPI2				(13)
#define GPI3				(14)
#define GPI4				(15)
#define GPI5				(16)
#define GPG5				(17)
enum{
	EDID_DEFAULT_FHD 	= 0x01,
	EDID_DEFAULT_4K2K = 0x02,
	EDID_COPY			= 0x03,
	EDID_COMPOSE_MIN	= 0x04,
	EDID_COMPOSE_J	= 0x05,
};
enum{
	HDMI_TX_A = 0x00,
	HDMI_TX_B = 0x01,
	HDMI_TX_C = 0x02,
	HDMI_TX_D = 0x03,
};

#define IT6662_HPD_H				(0x03)
#define IT6662_HPD_L				(0x01)
#define IT6662_HPD_AUTO			(0x00)

#define IT6662_CEC_A				(0x80 | (HDMI_TX_A << 4))
#define IT6662_CEC_B				(0x80 | (HDMI_TX_B << 4))
#define IT6662_CEC_C				(0x80 | (HDMI_TX_C << 4))
#define IT6662_CEC_D				(0x80 | (HDMI_TX_D << 4))


typedef enum _tagLINK_PIXEL {
    LINK_PIXEL_QCIF_25 = 0,        /*QCIF      176x144*/
	LINK_PIXEL_QCIF_30,        /*QCIF      176x144*/
	LINK_PIXEL_QCIF_50,        /*QCIF      176x144*/
	LINK_PIXEL_QCIF_60,        /*QCIF      176x144*/	
    LINK_PIXEL_CIF_25,            /*480I      352x288*/
    LINK_PIXEL_CIF_30,            /*480I      352x288*/
    LINK_PIXEL_CIF_50,            /*480I      352x288*/
    LINK_PIXEL_CIF_60,            /*480I      352x288*/
    LINK_PIXEL_WCIF_25,           /*480I      512x288*/
    LINK_PIXEL_WCIF_30,           /*480I      512x288*/
    LINK_PIXEL_WCIF_50,           /*480I      512x288*/
    LINK_PIXEL_WCIF_60,           /*480I      512x288*/
    LINK_PIXEL_4CIF_25,           /*480I      704x576*/
    LINK_PIXEL_4CIF_30,           /*480I      704x576*/
    LINK_PIXEL_4CIF_50,           /*480I      704x576*/
    LINK_PIXEL_4CIF_60,           /*480I      704x576*/
    LINK_PIXEL_288P_25,           /*480I      360x288*/
    LINK_PIXEL_288P_30,           /*480I      360x288*/
    LINK_PIXEL_288P_50,           /*480I      360x288*/
    LINK_PIXEL_288P_60,           /*480I      360x288*/
    LINK_PIXEL_360P_25,      //5  /*360P      480x360*/
    LINK_PIXEL_360P_30,      //5  /*360P      480x360*/
    LINK_PIXEL_360P_50,      //5  /*360P      480x360*/
    LINK_PIXEL_360P_60,      //5  /*360P      480x360*/
    LINK_PIXEL_480I_25,           /*480I      704x480*/
    LINK_PIXEL_480I_30,           /*480I      704x480*/
    LINK_PIXEL_480I_50,           /*480I      704x480*/
    LINK_PIXEL_480I_60,           /*480I      704x480*/
    LINK_PIXEL_480P_25,            /*480P      704x480*/
    LINK_PIXEL_480P_30,            /*480P      704x480*/
    LINK_PIXEL_480P_50,            /*480P      704x480*/
    LINK_PIXEL_480P_60,            /*480P      704x480*/
    LINK_PIXEL_576I_25,            /*576I      704x576*/
    LINK_PIXEL_576I_30,            /*576I      704x576*/
    LINK_PIXEL_576I_50,            /*576I      704x576*/
    LINK_PIXEL_576I_60,            /*576I      704x576*/
    LINK_PIXEL_576P_25,            /*576P      704x576*/
    LINK_PIXEL_576P_30,            /*576P      704x576*/
    LINK_PIXEL_576P_50,            /*576P      704x576*/
    LINK_PIXEL_576P_60,            /*576P      704x576*/
    LINK_PIXEL_720P_25,      //10  /*720P      1280x720*/
    LINK_PIXEL_720P_30,      //10  /*720P      1280x720*/
    LINK_PIXEL_720P_50,      //10  /*720P      1280x720*/
    LINK_PIXEL_720P_60,      //10  /*720P      1280x720*/
    LINK_PIXEL_1080I_25,           /*1080I     1920x1080*/
    LINK_PIXEL_1080I_30,           /*1080I     1920x1080*/
    LINK_PIXEL_1080I_50,           /*1080I     1920x1080*/
    LINK_PIXEL_1080I_60,           /*1080I     1920x1080*/
    LINK_PIXEL_DW_1080P_25,        /*double width for two 1080P     1792x2016*/
    LINK_PIXEL_DW_1080P_30,        /*double width for two 1080P     1792x2016*/
    LINK_PIXEL_DW_1080P_50,        /*double width for two 1080P     1792x2016*/
    LINK_PIXEL_DW_1080P_60,        /*double width for two 1080P     1792x2016*/
    LINK_PIXEL_1080P_25,     //13  /*1080P     1920x1080*/
    LINK_PIXEL_1080P_30,     //13  /*1080P     1920x1080*/
    LINK_PIXEL_1080P_50,     //13  /*1080P     1920x1080*/
    LINK_PIXEL_1080P_60,     //13  /*1080P     1920x1080*/
    LINK_PIXEL_VGA_25,             /*VGA       640x480*/
    LINK_PIXEL_VGA_30,             /*VGA       640x480*/
    LINK_PIXEL_VGA_50,             /*VGA       640x480*/
    LINK_PIXEL_VGA_60,             /*VGA       640x480*/
    LINK_PIXEL_SVGA_25,            /*SVGA      800x600*/
    LINK_PIXEL_SVGA_30,            /*SVGA      800x600*/
    LINK_PIXEL_SVGA_50,            /*SVGA      800x600*/
    LINK_PIXEL_SVGA_60,            /*SVGA      800x600*/
    LINK_PIXEL_XGA_25,             /*XGA       1024x768*/
    LINK_PIXEL_XGA_30,             /*XGA       1024x768*/
    LINK_PIXEL_XGA_50,             /*XGA       1024x768*/
    LINK_PIXEL_XGA_60,             /*XGA       1024x768*/
    LINK_PIXEL_WXGA_1280_768_25,   /*WXGA      1280x768*/
    LINK_PIXEL_WXGA_1280_768_30,   /*WXGA      1280x768*/
    LINK_PIXEL_WXGA_1280_768_50,   /*WXGA      1280x768*/
    LINK_PIXEL_WXGA_1280_768_60,   /*WXGA      1280x768*/
    LINK_PIXEL_WXGA_1280_800_25,   /*WXGA      1280x800*/
    LINK_PIXEL_WXGA_1280_800_30,   /*WXGA      1280x800*/
    LINK_PIXEL_WXGA_1280_800_50,   /*WXGA      1280x800*/
    LINK_PIXEL_WXGA_1280_800_60,   /*WXGA      1280x800*/
    LINK_PIXEL_WXGA_1366_768_25,   /*WXGA      1366x768*/
    LINK_PIXEL_WXGA_1366_768_30,   /*WXGA      1366x768*/
    LINK_PIXEL_WXGA_1366_768_50,   /*WXGA      1366x768*/
    LINK_PIXEL_WXGA_1366_768_60,   /*WXGA      1366x768*/
    LINK_PIXEL_QUADVGA_25,         /*Quad-VGA  1280x960*/
    LINK_PIXEL_QUADVGA_30,         /*Quad-VGA  1280x960*/
    LINK_PIXEL_QUADVGA_50,         /*Quad-VGA  1280x960*/
    LINK_PIXEL_QUADVGA_60,         /*Quad-VGA  1280x960*/
    LINK_PIXEL_SXGA_25,            /*SXGA      1280x1024*/
    LINK_PIXEL_SXGA_30,            /*SXGA      1280x1024*/
    LINK_PIXEL_SXGA_50,            /*SXGA      1280x1024*/
    LINK_PIXEL_SXGA_60,            /*SXGA      1280x1024*/
    LINK_PIXEL_WXGAU_25,           /*WXGA+     1440x900*/
    LINK_PIXEL_WXGAU_30,           /*WXGA+     1440x900*/
    LINK_PIXEL_WXGAU_50,           /*WXGA+     1440x900*/
    LINK_PIXEL_WXGAU_60,           /*WXGA+     1440x900*/
    LINK_PIXEL_SXGAU_25,           /*SXGA+     1440x1050*/
    LINK_PIXEL_SXGAU_30,           /*SXGA+     1440x1050*/
    LINK_PIXEL_SXGAU_50,           /*SXGA+     1440x1050*/
    LINK_PIXEL_SXGAU_60,           /*SXGA+     1440x1050*/
    LINK_PIXEL_WXGAUU_25,          /*WXGA++    1600x900*/
    LINK_PIXEL_WXGAUU_30,          /*WXGA++    1600x900*/
    LINK_PIXEL_WXGAUU_50,          /*WXGA++    1600x900*/
    LINK_PIXEL_WXGAUU_60,          /*WXGA++    1600x900*/
    LINK_PIXEL_WSXGAU_25,          /*WSXGA+    1680x1050*/
    LINK_PIXEL_WSXGAU_30,          /*WSXGA+    1680x1050*/
    LINK_PIXEL_WSXGAU_50,          /*WSXGA+    1680x1050*/
    LINK_PIXEL_WSXGAU_60,          /*WSXGA+    1680x1050*/
    LINK_PIXEL_UXGA_25,            /*UXGA      1600x1200*/
    LINK_PIXEL_UXGA_30,            /*UXGA      1600x1200*/
    LINK_PIXEL_UXGA_50,            /*UXGA      1600x1200*/
    LINK_PIXEL_UXGA_60,            /*UXGA      1600x1200*/
    LINK_PIXEL_WUXGA_25,           /*WUXGA     1920x1200*/
    LINK_PIXEL_WUXGA_30,           /*WUXGA     1920x1200*/
    LINK_PIXEL_WUXGA_50,           /*WUXGA     1920x1200*/
    LINK_PIXEL_WUXGA_60,           /*WUXGA     1920x1200*/
    LINK_PIXEL_QXGA_25,            /*QXGA      2048x1536*/
    LINK_PIXEL_QXGA_30,            /*QXGA      2048x1536*/
    LINK_PIXEL_QXGA_50,            /*QXGA      2048x1536*/
    LINK_PIXEL_QXGA_60,            /*QXGA      2048x1536*/
    LINK_PIXEL_WQXGA_25,           /*WQXGA     2560x1600*/
    LINK_PIXEL_WQXGA_30,           /*WQXGA     2560x1600*/
    LINK_PIXEL_WQXGA_50,           /*WQXGA     2560x1600*/
    LINK_PIXEL_WQXGA_60,           /*WQXGA     2560x1600*/
    LINK_PIXEL_2KP_25,      /*          1920x2160*/
    LINK_PIXEL_2KP_30,      /*          1920x2160*/
    LINK_PIXEL_2KP_50,      /*          1920x2160*/
    LINK_PIXEL_2KP_60,      /*          1920x2160*/
    LINK_PIXEL_4K2KP_25,           /*4K2K      3840x2160*/
    LINK_PIXEL_4K2KP_30,           /*4K2K      3840x2160*/
    LINK_PIXEL_4K2KP_50,           /*4K2K      3840x2160*/
    LINK_PIXEL_4K2KP_60,           /*4K2K      3840x2160*/
    LINK_PIXEL_CUSTOM,          /*�Զ���*/
    LINK_PIXEL_BUTT
} LINK_PIXEL;


#endif
