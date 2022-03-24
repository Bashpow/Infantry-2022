#ifndef __DETECT_TASK_H
#define __DETECT_TASK_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

typedef enum 
{
	REMOTE_CONTROL = 0u, //ң����
	CHASSIS_MOTOR,       //���̵��
	GIMBAL_MOTOR,        //��̨���
	SHOOTER_MOTOR,       //����������
	AUTO_AIM,            //����NUC
	JUDGE_SYSTEM,        //����ϵͳ
	SUPER_CAPACITOR,     //��������
	GYROSCOPE            //������
}errorList;

extern TaskHandle_t DetectTask_Handler;
void Detect_Task(void *pvParameters);
uint8_t Get_Module_Online_State(uint8_t id);
void Detect_Reload(uint8_t index);
void Classis_Reload(int8_t motor_index);
void Gimbal_Reload(int8_t motor_index);

#endif
