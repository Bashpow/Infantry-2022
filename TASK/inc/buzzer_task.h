#ifndef __BUZZER_TASK
#define __BUZZER_TASK

#include "stm32f4xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

extern TaskHandle_t BuzzerTask_Handler;

void Buzzer_Task(void *pvParameters);

BaseType_t Set_Beep_Time(uint8_t send_msg, uint16_t pwm, uint16_t time);

#endif
