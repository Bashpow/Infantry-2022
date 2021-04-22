#ifndef __CAN1_H
#define __CAN1_H

#include "stm32f4xx.h"

void Can1_Init(void);
void Can1_Send_4Msg(uint32_t id, int16_t data1, int16_t data2, int16_t data3, int16_t data4);


#endif
