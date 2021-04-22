#ifndef __GIMBAL_TASK_H
#define __GIMBAL_TASK_H

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t GimbalTask_Handler;

void Gimbal_Task(void *pvParameters);


#endif
