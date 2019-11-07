#ifndef _SPI_H
#define _SPI_H

//#include "stm32f10x.h"

extern void spi_init(void);
extern u8   SPI_SendString(u8 *pData, u16 len);
extern u8   SPI_ReciveString(u8 *pData, u16 len);
extern void SPI_SetSNNHigh(void);
extern void SPI_SetSNNLow(void);

void SPI2_init(void);

#endif

