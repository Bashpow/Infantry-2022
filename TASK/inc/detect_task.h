#ifndef __DETECT_TASK_H
#define __DETECT_TASK_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

typedef enum 
{
	REMOTE_CONTROL = 0u, //遥控器
	CHASSIS_MOTOR,       //底盘电机
	GIMBAL_MOTOR,        //云台电机
	SHOOTER_MOTOR,       //发射机构电机
	AUTO_AIM,            //自瞄NUC
	JUDGE_SYSTEM,        //裁判系统
	SUPER_CAPACITOR,     //超级电容
	GYROSCOPE            //陀螺仪
}errorList;

extern TaskHandle_t DetectTask_Handler;
void Detect_Task(void *pvParameters);
uint8_t Get_Module_Online_State(uint8_t id);
void Detect_Reload(uint8_t index);
void Classis_Reload(int8_t motor_index);
void Gimbal_Reload(int8_t motor_index);

#endif
