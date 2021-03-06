/***********************************************************************************
工程名称: NA7600_STM32_INFLASH.c
创建时间: 2015-7-28
工程内容: 主要实现 往STM32的内部FLASH中存储数据
注意事项: 根据所选的芯片来修改所选的 STM32_FLASH_SIZE、STM_SECTOR_SIZE、FLASH_SAVE_ADDR

************************************************************************************/
#include "flash.h"
#include "debug_usart.h"

#if STM32_FLASH_WREN	//如果使能了写   

/*
	函数功能: 	不检查的写入
  	输入参数:	WriteAddr: 起始地址
				pBuffer: 数据指针
				NumToWrite: 半字(16位)数  
*/
void STMFLASH_Write_NoCheck(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite)   
{ 			 		 
	u16 i;
	
	for(i = 0; i < NumToWrite; i++)
	{
		FLASH_ProgramHalfWord(WriteAddr, pBuffer[i]);  //在指定的地址写入半字
	    WriteAddr += 2;  //地址增加2.
	}  
} 

//看手册可知大容量芯片的每个扇区是2K大小，而小容量产品每个扇区是1K大小
#if STM32_FLASH_SIZE < 256

	#define STM_SECTOR_SIZE 1024  //字节
	
#else 

	#define STM_SECTOR_SIZE	2048

#endif		 

u16 STMFLASH_BUF[STM_SECTOR_SIZE / 2];  //定义数组最大为2K字节

/*
	函数功能: 	从指定地址开始写入指定长度的数据
  	输入参数:	WriteAddr: 起始地址(此地址必须为2的倍数!!)
				pBuffer: 数据指针
				NumToWrite:半字(16位)数 (就是要写入的16位数据的个数.) 
*/

void bsp_flashEraseStartPage(u8 startPage)
{
    u32 i = 0;

    for(i = startPage; i <= 255; i++)
    {
        FLASH_ErasePage(i * STM_SECTOR_SIZE + STM32_FLASH_BASE);
    }
}

#if 0
void STMFLASH_Write(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite)	
{
	u16 i;
	u32 secpos;	   //扇区地址
	u16 secoff;	   //扇区内偏移地址(16位字计算)
	u16 secremain; //扇区内剩余地址(16位字计算)	       
	u32 offaddr;   //去掉0X08000000后的地址

	//先判断地址是否合法
	if(WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
	{
		return;  
	}
	
	FLASH_Unlock();	 //解锁
	
	offaddr = WriteAddr - STM32_FLASH_BASE;		//实际偏移地址.
	secpos = offaddr / STM_SECTOR_SIZE;			//扇区地址  0~255
	secoff = (offaddr % STM_SECTOR_SIZE) / 2;   //在扇区内的偏移(u16占两个字节，故以2个字节为基本单位.)
	secremain = STM_SECTOR_SIZE / 2 -secoff;	//扇区剩余空间大小(u16 占2个字节)  
	
	if(NumToWrite <= secremain)
	{
		secremain = NumToWrite;  //不大于该扇区范围
	}
	
	while(1) 
    {
        STMFLASH_Write_NoCheck(WriteAddr, pBuffer, secremain);  //扇区已经擦除了的,直接写入扇区剩余区间. 	
		
		if(NumToWrite == secremain)
		{
			break;  //写入结束了
		}
		else  //写入未结束
		{
			secpos++;	//扇区地址增1
			secoff = 0;  //偏移位置为0 	 
		   	pBuffer += secremain;  	//指针偏移
			WriteAddr += secremain;	 //写地址偏移	   
		   	NumToWrite -= secremain;  //字节(16位)数递减
			
			if(NumToWrite > (STM_SECTOR_SIZE / 2))
			{
				secremain = STM_SECTOR_SIZE / 2;  //下一个扇区还是写不完
			}
			else 
			{
				secremain = NumToWrite;  //下一个扇区可以写完了
			}
		}	 
	}	
	
	FLASH_Lock();  //上锁
}
#else
void STMFLASH_Write(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite)	
{
	u16 i;
	u32 secpos;	   //扇区地址
	u16 secoff;	   //扇区内偏移地址(16位字计算)
	u16 secremain; //扇区内剩余地址(16位字计算)	       
	u32 offaddr;   //去掉0X08000000后的地址

	//先判断地址是否合法
	if(WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
	{
		return;  
	}
	
	FLASH_Unlock();	 //解锁
	
	offaddr = WriteAddr - STM32_FLASH_BASE;		//实际偏移地址.
	secpos = offaddr / STM_SECTOR_SIZE;			//扇区地址  0~127 for STM32F103RBT6
	secoff = (offaddr % STM_SECTOR_SIZE) / 2;   //在扇区内的偏移(u16占两个字节，故以2个字节为基本单位.)
	secremain = STM_SECTOR_SIZE / 2 -secoff;	//扇区剩余空间大小(u16 占2个字节)  
	
	if(NumToWrite <= secremain)
	{
		secremain = NumToWrite;  //不大于该扇区范围
	}
	
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2);  //读出整个扇区的内容
		
		for(i = 0; i < secremain; i++)  //校验数据是否为0xffff,STM32内部FLASH在写扇区前也要保证所写扇区被擦除，即值为0xffff
		{
			if(STMFLASH_BUF[secoff + i] != 0XFFFF)
			{
				break;  //需要擦除  	 
			} 
		}
		
		if(i < secremain)  //需要擦除
		{
			FLASH_ErasePage(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE);  //擦除这个扇区
			
			for(i = 0; i < secremain; i++)  //复制
			{
				STMFLASH_BUF[i+secoff] = pBuffer[i];	  
			}
			
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2);  //写入整个扇区  
		}
		else 
		{
			STMFLASH_Write_NoCheck(WriteAddr, pBuffer, secremain);  //扇区已经擦除了的,直接写入扇区剩余区间. 	
		}
		
		if(NumToWrite == secremain)
		{
			break;  //写入结束了
		}
		else  //写入未结束
		{
			secpos++;	//扇区地址增1
			secoff = 0;  //偏移位置为0 	 
		   	pBuffer += secremain;  	//指针偏移
			WriteAddr += secremain;	 //写地址偏移	   
		   	NumToWrite -= secremain;  //字节(16位)数递减
			
			if(NumToWrite > (STM_SECTOR_SIZE / 2))
			{
				secremain = STM_SECTOR_SIZE / 2;  //下一个扇区还是写不完
			}
			else 
			{
				secremain = NumToWrite;  //下一个扇区可以写完了
			}
		}	 
	}	
	
	FLASH_Lock();  //上锁
}

#endif
#endif

/*
	函数功能:	读取指定地址的半字(16位数据)
  	输入参数:	faddr:读地址(此地址必须为2的倍数!!)
	返回值: 	对应数据
*/
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}

/*
	函数功能: 	从指定地址开始读出指定长度的数据
  	输入参数:	ReadAddr: 起始地址
				pBuffer: 数据指针
				NumToRead:半字(16位)数 
*/
void STMFLASH_Read(u32 ReadAddr, u16 *pBuffer, u16 NumToRead)
{
	u16 i;
	
	for(i = 0; i < NumToRead; i++)
	{
		pBuffer[i] = STMFLASH_ReadHalfWord(ReadAddr);  //读取2个字节.
		ReadAddr += 2;  //偏移2个字节.	
	}
}

/*
	函数功能: 	测试函数
  	输入参数:	WriteAddr:起始地址
				WriteData:要写入的数据
*/
void STMFLASH_WtiteU16DataToFlash(u32 WriteAddr, u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData, 1);  //写入一个字 
}






