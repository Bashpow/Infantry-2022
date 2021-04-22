#ifndef __AUTOAIM_TASK_H
#define __AUTOAIM_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

extern TaskHandle_t AutoaimTask_Handler;

void Autoaim_Task(void *pvParameters);

void Autoaim_Data_Update(void);

#endif
