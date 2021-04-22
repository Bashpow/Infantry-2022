#ifndef __BUZZER_H
#define __BUZZER_H

#include "stm32f4xx.h"

#define BUZZER_ON(pwm) {TIM12->ARR=pwm;TIM_SetCompare1(TIM12, pwm/2);}
#define BUZZER_OFF() TIM_SetCompare1(TIM12, 0);

void Buzzer_Init(void);


#endif
