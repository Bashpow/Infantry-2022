#ifndef __SHOOTER_TASK_H
#define __SHOOTER_TASK_H


#include "FreeRTOS.h"
#include "task.h"


extern TaskHandle_t ShooterTask_Handler;

void Shooter_Task(void *pvParameters);

void Shooter_Friction_Speed_Limit(void);
void First_Shoot_Friction_Speed_Subtract(uint16_t minus_speed);

#endif
