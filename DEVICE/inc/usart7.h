#ifndef __USART7_H
#define __USART7_H

#include "stm32f4xx.h"

void Uart7_Init(void);

u8 Get_Uart7_Available_Bufferx(void);
const u8* Get_Uart7_Bufferx(u8 bufx);
void Uart7_DMA_Reset(void);

#endif
