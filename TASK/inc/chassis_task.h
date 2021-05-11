#ifndef __CHASSIS_TASK_H
#define __CHASSIS_TASK_H

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t ChassisTask_Handler;

void Chassis_Task(void *pvParameters);

void Change_Chassis_Motor_Boost_Rate(int8_t mode, float max_rate);

#endif

