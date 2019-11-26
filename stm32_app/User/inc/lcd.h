#ifndef __LCD_H
#define __LCD_H

#if 0
#define CS_1             GPIO_SetBits(GPIOD, GPIO_Pin_3)    //取消LCD片选
#define CS_0             GPIO_ResetBits(GPIOD, GPIO_Pin_3)  //LCD片选有效
#define RES_1            GPIO_SetBits(GPIOD, GPIO_Pin_4)    //LCD低电平复位，复位之后保持高电平
#define RES_0            GPIO_ResetBits(GPIOD, GPIO_Pin_4)  //LCD复位
#define A0_1             GPIO_SetBits(GPIOD, GPIO_Pin_5)   //Ao=1,向LCD输入数据
#define A0_0             GPIO_ResetBits(GPIOD, GPIO_Pin_5) //Ao=0,向LCD写入指令
#define WR_1             GPIO_SetBits(GPIOD, GPIO_Pin_11)   //LCD写禁止
#define WR_0             GPIO_ResetBits(GPIOD, GPIO_Pin_11) //LCD写允许
#define RD_1             GPIO_SetBits(GPIOD, GPIO_Pin_12)   //LCD读禁止
#define RD_0             GPIO_ResetBits(GPIOD, GPIO_Pin_12) //LCD读允许
#endif

extern void lcd_init(void);
extern void lcd_display_ip(char *ip_address, u8 StartPage, u8 column);
extern void lcd_display_mainSlave(char *src, u8 StartPage, u8 column);
extern void lcd_clearScreen(void);

#endif

