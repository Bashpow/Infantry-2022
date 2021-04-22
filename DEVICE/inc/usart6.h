#ifndef __USART6_H
#define __USART6_H

#include "stm32f4xx.h"

void Usart6_Init(void);
void Usart6_DMA_Reset(void);

const uint8_t *Get_Usart6_Rx_Buf(void);
uint8_t Get_Usart6_Rx_Flag(void);
void Reset_Usart6_Rx_Flag(void);

#endif
