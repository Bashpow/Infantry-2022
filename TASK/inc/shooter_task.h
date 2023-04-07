#ifndef __SHOOTER_TASK_H
#define __SHOOTER_TASK_H


#include "FreeRTOS.h"
#include "task.h"


extern TaskHandle_t ShooterTask_Handler;

void Shooter_Task(void *pvParameters);
void Set_Shoot_key(u8 key);
void JudgeSystemGetShootMsgCallback(unsigned short id);
void ShooterFriction3508SpeedBaseLimit(uint16_t speed_limit);
int32_t ShooterSpeedToFrictionSpeed(int16_t shooter_speed);

void Shooter_Friction_Speed_Base_Limit(uint16_t speed_limit);
void Shooter_Friction_Speed_Limit(void);

#endif
