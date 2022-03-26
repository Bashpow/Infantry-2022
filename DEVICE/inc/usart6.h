#ifndef __USART6_H_
#define __USART6_H_

#include "stm32f4xx.h"

void Usart6_Init(void);
void Usart6_DMA_Reset(void);

const uint8_t *Get_Usart6_Rx_Buf(void);
uint8_t Get_Usart6_Rx_Flag(void);
void Reset_Usart6_Rx_Flag(void);
void Usart6_Send_Buf(uint8_t *buffer, uint32_t len);

#endif //__USART6_H_
