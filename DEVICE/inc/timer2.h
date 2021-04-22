#ifndef __TIMER2_H
#define __TIMER2_H

#include "stm32f4xx.h"

#define PA0_PWM_OUT(ccr)  TIM2->CCR1 = ccr;
#define PA1_PWM_OUT(ccr)  TIM2->CCR2 = ccr;
#define PA2_PWM_OUT(ccr)  TIM2->CCR3 = ccr;
#define PA3_PWM_OUT(ccr)  TIM2->CCR4 = ccr;

#define S_PWM_OUT(ccr) PA0_PWM_OUT(ccr)
#define T_PWM_OUT(ccr) PA1_PWM_OUT(ccr)
#define U_PWM_OUT(ccr) PA2_PWM_OUT(ccr)
#define V_PWM_OUT(ccr) PA3_PWM_OUT(ccr)

void TIM2_PWM_Init(u32 arr, u32 psc);

#endif
