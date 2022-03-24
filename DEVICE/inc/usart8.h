#ifndef __USART8_H
#define __USART8_H

#include "stm32f4xx.h"

void Usart8_Init(void);

const uint8_t* Get_Judge_Buf(void);
u8 Get_Judge_Buf_Len(void);
uint8_t Uart8_Transmit_Start(uint32_t tx_buffer_address, uint16_t tx_buffer_len);

#endif
