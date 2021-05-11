#ifndef __REMOTER_TASK_H
#define __REMOTER_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "remoter.h"

typedef struct
{
	u8 control_device; //�����豸 1����ģʽ 2ң����ģʽ
	u8 rc_motion_mode; //������̨ģʽ 1���̸��� 2С���� 3������� 4����С���� 5����
	u8 mouse_key_chassis_mode; //1���� 2С���� 3����
	u8 mouse_key_gimbal_mode; //1�ֶ� 2���� 3����
	u8 shoot_mode; //���ģʽ 1���� 2������ 3�����ٶ�һ 4�����ٶȶ�
	u8 fric_cover_mode; //Ħ����ģʽ 0�ر� 1���� 2���� 3���ո�
	
}Robot_mode_t;



extern TaskHandle_t RemoterTask_Handler;

void Remoter_Task(void *pvParameters);
void Rc_Data_Update(void);
void Fric_Reset(void);

const Rc_ctrl_t *Get_Remote_Control_Point(void);
const Robot_mode_t *Get_Robot_Mode_Point(void);


#endif

