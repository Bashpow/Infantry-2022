#ifndef __DETECT_TASK_H
#define __DETECT_TASK_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t DetectTask_Handler;
void Detect_Task(void *pvParameters);
u8 Get_Module_Online_State(u8 id);
void Detect_Reload(u8 index);
void Classis_Reload(int8_t motor_index);
void Gimbal_Reload(int8_t motor_index);
void Shooter_Reload(void);

#endif
