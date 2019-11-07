#include "stm32f10x.h"
#include "ds1339c.h"
#include "myiic.h"
#include "delay.h"
#include "debug_usart.h"
#include "myiic.h"

#define DS1339C_ADDR_W				0xD0	// 写地址
#define DS1339C_ADDR_R				0xD1	// 读地址
#define SEC_REG						0x00
#define MINUTES_REG				    0x01
#define HOUR_REG					0x02
#define WEEK_REG					0x03	// 周 对应手册 day
#define DAY_REG						0x04	// 天 对应手册 date
#define MONTH_REG					0x05	
#define YEAR_REG					0x06
#define ALARM1_SEC_REG				0x07
#define ALARM1_MINUTES_REG			0x08
#define ALARM1_HOUR_REG				0x09
#define ALARM1_WEEK_DAY_REG			0x0A	// 周、天
#define ALARM2_MINUTES_REG			0x0B
#define ALARM2_HOUR_REG				0x0C
#define ALARM2_WEEK_DAY_REG			0x0D	// 周、天
#define CONTROL_REG					0x0E
#define STATUS_REG					0x0F
#define TRICKLE_CHARGER_REG			0x10

//#define CONTROL_REG_VALUE			0x18	// 寄存器设置的值	
#define CONTROL_REG_VALUE			0x20	// 寄存器设置的值	
#define STATUS_REG_VALUE			0x00
#define TRICKLE_CHARGER_REG_VALUE	0x00

/* 占寄存器的位 */
#define SEC_BIT						0x7F	// 秒在寄存器中的位置(这里占前面7位)
#define MIN_BIT						0x7F
#define HOUR_BIT					0x3F
#define AMPM_BIT                    0x20
#define PM_BIT					    0x20	/* BIT5--0: AM;		 --1: PM */
#define AM_BIT					    0xdf	/* BIT5--0: AM;		 --1: PM */
#define HOUR_12_24_BIT				0x40	/* BIT6--1: 12 hour; --0: 24 hour  */
#define HOUR_12_BIT					0x1F
#define HOUR_24_BIT					0x3F
#define DAY_BIT						0x3F
#define WEEK_BIT					0x07
#define MONTH_BIT					0x1F
#define CENTURY_BIT					0x80

#define AM_VALUE					0
#define PM_VALUE					1

#define DataToBcd(x) (((x)/10)*16+((x)%10))
#define BcdToData(x) (((x)/16)*10+((x)%16))

extern unsigned char reg_val[I2C_REG_NUM]; 

typedef struct
{	
	u8 sec;
	u8 min;
	u8 hour;
	u8 day;
	u8 mon;
	u8 week;
	u16 year;
}DATE;

/* 先发送写然后发送写的地址，这样就可以改变地址指针的位置，
 * 然后再进入度模式，就可以读到ReadAddr寄存器中的值 */
static u8 ds1339c_readOneByte(u8 ReadAddr)
{
	u8 temp = 0;
	
	IIC_Start(IIC_E_BUS0);
	IIC_SendByte(IIC_E_BUS0, DS1339C_ADDR_W);
	IIC_WaitAck(IIC_E_BUS0);
	IIC_SendByte(IIC_E_BUS0, ReadAddr);
	IIC_WaitAck(IIC_E_BUS0);
	IIC_Stop(IIC_E_BUS0);
	//delay_ms(10);
	IIC_Start(IIC_E_BUS0);
	IIC_SendByte(IIC_E_BUS0, DS1339C_ADDR_R);
	IIC_WaitAck(IIC_E_BUS0);
	temp = IIC_ReadByte(IIC_E_BUS0, 0);
	IIC_Stop(IIC_E_BUS0);
	//DebugPrint("temp = 0x%x \n", temp);
	return temp;
}

static void ds1339c_writeOneByte(u8 WriteAddr, u8 DataToWrite)
{
	IIC_Start(IIC_E_BUS0);
	IIC_SendByte(IIC_E_BUS0, DS1339C_ADDR_W);
	IIC_WaitAck(IIC_E_BUS0);
	IIC_SendByte(IIC_E_BUS0, WriteAddr);
	IIC_WaitAck(IIC_E_BUS0);
	IIC_SendByte(IIC_E_BUS0, DataToWrite);
	IIC_WaitAck(IIC_E_BUS0);
	IIC_Stop(IIC_E_BUS0);
	delay_ms(10);
}

/* 检测数据是否有效 */
#if 0
/**
 * 判断是否是闰年函数
 * 月份   1  2  3  4  5  6  7  8  9  10 11 12
 * 闰年   31 29 31 30 31 30 31 31 30 31 30 31
 * 非闰： 31 28 31 30 31 30 31 31 30 31 30 31
 * year:年份
 * 返回1：闰年		0：不是闰年
 */
static u8 is_leap_year(u16 year)
{			  
	if (year%4==0)	// 必须能被4整除
	{ 
		if (year%100==0) 
		{ 
			if (year%400==0) // 如果以00结尾，同时被400整除，是闰年
			{
				return 1;		   
			}
			else 
			{
				return 0;   
			}
		} 
		else 
		{
			return 1;   
		}
	} 
	else 
	{
		return 0;	
	}
}

/**
 * return -1: input date format ERROR
 * return 0 : input date format right
 */
static int check_monValid(const DATE * const date)
{
	if ((date->mon < 1) || (date->mon > 12))
		goto err;
	
	if (!(date->mon % 2)) 
	{
		if (2 == date->mon)
		{
			if (is_leap_year(date->year)) 
			{
				if (date->day < 1 || date->day > 29)
					goto err;
			} 
			else 
			{
				if ((date->day < 1) || (date->day > 28))
					goto err;				
			}
		} 
		else 
		{
			if ((date->day < 1) || (date->day > 30)) 
				goto err;
		}
	}
	else 
	{
		if ((date->day < 1) || (date->day > 31))
			goto err;				
	}

	return 0;
err:
	DebugPrint("date format err \n");
	return -1;	
}

/**
 * return -1: input date format ERROR
 * return 0 : input date format right
 */
static int check_dateValid(const DATE * const date)
{
	if (check_monValid(date))
		goto err;

	// 这里century没有处理? TODO
	if (!(date->sec > 0 && date->sec < 59) || (!(date->min < 59 && date->min > 0))
		|| (date->year > 2100 || date->year < 2000) || (date->week > 7 || date->week < 1)) 
	{
		goto err;
	}

	if (ds1339c_readOneByte(HOUR_REG) & HOUR_12_24_BIT)
	{
		if (date->hour > 12 || date->hour < 1) 
			goto err;
	}
	else 
	{
		if (!(date->hour < 23 && date->hour > 0)) 
		{
			goto err;
		}
	}
		
	return 0;
err:
	DebugPrint("date format err \n");
	return -1;
}
#endif

/**
 * return -1: input date format ERROR
 * return 0 : set successful
 */
static int ds1339c_setDate(void)
{
	//这里是否有写保护 TODO
	u8 sec, min, hour, day, mon;

	//check_dateValid();	//数据有检查 TODO
	sec = DataToBcd((reg_val[SYS_RTC_SEC_W]) & SEC_BIT);
	min = DataToBcd((reg_val[SYS_RTC_MIN_W]) & MIN_BIT);
	//hour = ds1339c_readOneByte((HOUR_REG) & HOUR_12_24_BIT);
	if (reg_val[SYS_RTC_12_24_W])
	{
		if(reg_val[SYS_RTC_AMPM_W])
		{
			hour = DataToBcd( reg_val[SYS_RTC_HOUR_W] ) | PM_BIT | HOUR_12_24_BIT ;
		}
		else
		{
			hour = DataToBcd(( reg_val[SYS_RTC_HOUR_W] ) & AM_BIT) | HOUR_12_24_BIT ;
		}
	} 
	else
	{
		hour = DataToBcd((reg_val[SYS_RTC_HOUR_W] & HOUR_BIT));
	}	
	day = DataToBcd((reg_val[SYS_RTC_DAY_W]) & DAY_BIT);
	//mon = (ds1339c_readOneByte(MONTH_REG) & CENTURY_BIT);
	mon = DataToBcd((reg_val[SYS_RTC_MONTH_W]) & MONTH_BIT);

	ds1339c_writeOneByte(HOUR_REG, hour);
	ds1339c_writeOneByte(DAY_REG, day);
	ds1339c_writeOneByte(MONTH_REG, mon);
	ds1339c_writeOneByte(WEEK_REG, DataToBcd(reg_val[SYS_RTC_WEEK_W] & WEEK_BIT));
	ds1339c_writeOneByte(YEAR_REG, DataToBcd(reg_val[SYS_RTC_YEAR_W]));
	ds1339c_writeOneByte(MINUTES_REG, min);
	ds1339c_writeOneByte(SEC_REG, sec);

	return 0;
}

static void ds1339c_readDate(void)
{
	u8 temp = 0;
	reg_val[SYS_RTC_SEC]  = BcdToData(ds1339c_readOneByte(SEC_REG) & SEC_BIT);
	reg_val[SYS_RTC_MIN]  = BcdToData(ds1339c_readOneByte(MINUTES_REG) & MIN_BIT);
	
	temp = ds1339c_readOneByte(HOUR_REG);
	if(temp & HOUR_12_24_BIT) //判断是12 or 24 小时制
	{
		reg_val[SYS_RTC_12_24] = 1;  //表示十二小时制
		reg_val[SYS_RTC_AMPM] = (temp & AMPM_BIT)>>5;
		/*
		if(temp & AMPM_BIT)
			{reg_val[SYS_RTC_AMPM] = 1;}  //PM
		else
			{reg_val[SYS_RTC_AMPM] = 0;}  //AM
		*/
		reg_val[SYS_RTC_HOUR] = BcdToData(temp & HOUR_12_BIT);
	}
	else
	{
		reg_val[SYS_RTC_12_24] = 0; //表示24小时制
		reg_val[SYS_RTC_AMPM] = 2; //表示在24小时模式
		reg_val[SYS_RTC_HOUR] = BcdToData(temp & HOUR_24_BIT);
	}
	reg_val[SYS_RTC_DAY]  = BcdToData(ds1339c_readOneByte(DAY_REG) & DAY_BIT);
	temp = ds1339c_readOneByte(MONTH_REG);
	reg_val[SYS_RTC_CENTURY] = temp & CENTURY_BIT;
	reg_val[SYS_RTC_MONTH] = BcdToData(temp & MONTH_BIT);
	reg_val[SYS_RTC_WEEK] = BcdToData(ds1339c_readOneByte(WEEK_REG) & WEEK_BIT);
	reg_val[SYS_RTC_YEAR] = BcdToData(ds1339c_readOneByte(YEAR_REG)); // 这里加上2000，即是实际年份
}

#if 0
 /**
  * reg_val[SYS_RTC_AMPM_W]--1: 12小时制,
  *						   --0：24小时制
  * note: 12小时转24小时制，需要重新设置hour
  *		  24小时转12小时制，会自动转换
  */
static void ds1339c_write_12_24(void)
{
	u8 temp = 0;
	u8 hour = 0;

	temp = ds1339c_readOneByte(HOUR_REG);
	if (reg_val[SYS_RTC_12_24_W])	// 12转24小时制，时间自动转换
	{
		if (!(temp & AMPM_BIT)) 
		{
			hour = (temp & HOUR_BIT);
			hour = BcdToData(hour);
			hour = hour % 12;
			if (0 == hour) 
			{
				hour = 0x0c;
			}
			hour = DataToBcd(hour) | AMPM_BIT;
			ds1339c_writeOneByte(HOUR_REG, hour);
		}
	} 
	else 
	{
		if (temp & AMPM_BIT)
		{
			ds1339c_writeOneByte(HOUR_REG, 0x00);
		}
	}
}
#endif

/* 检测寄存器，是否设置时间 */
void ds1339c_check_write_rtc(void)
{
	if (reg_val[SYS_RTC_CTL] & BIT0)
	{
		ds1339c_setDate();
		reg_val[SYS_RTC_CTL] &= ~BIT0;
	}	
}

/* 检测寄存器，是否读时间 */
void ds1339c_check_read_rtc(void)
{
	if (reg_val[SYS_RTC_CTL] & BIT1)
	{
		ds1339c_readDate();
		reg_val[SYS_RTC_CTL] &= ~BIT1;
	}
}

void ds1339c_init(void)
{ 
    ds1339c_writeOneByte(CONTROL_REG,CONTROL_REG_VALUE);
    ds1339c_writeOneByte(STATUS_REG, STATUS_REG_VALUE);
    ds1339c_writeOneByte(TRICKLE_CHARGER_REG, TRICKLE_CHARGER_REG_VALUE);
}

