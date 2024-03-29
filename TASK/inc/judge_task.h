#ifndef __JUDGE_TASK_H
#define __JUDGE_TASK_H

#include "stm32f4xx.h"

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t JudgeTask_Handler;

void Judge_Task(void *pvParameters);

void Notify_Judge_Task( uint32_t send_data );

#endif
