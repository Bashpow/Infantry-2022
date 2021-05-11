#ifndef __SHOOTER_TASK_H
#define __SHOOTER_TASK_H


#include "FreeRTOS.h"
#include "task.h"


extern TaskHandle_t ShooterTask_Handler;

void Shooter_Task(void *pvParameters);

void Set_Shoot_key(u8 key);

void Shooter_Friction_Speed_Limit(void);

#endif
