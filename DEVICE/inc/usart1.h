#ifndef __USART1_H
#define __USART1_H

#include "stm32f4xx.h"

void Usart1_Init(void);
const u8* Get_Rc_Bufferx(u8 bufx);
u8 Get_Rc_Available_Bufferx(void);
void Usart1_DMA_Reset(void);

#endif
