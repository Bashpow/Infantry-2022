#ifndef __CLIENTUI_TASK_H_
#define __CLIENTUI_TASK_H_

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t ClientuiTask_Handler;
void Clientui_Task(void *pvParameters);


#endif // __CLIENTUI_TASK_H_
