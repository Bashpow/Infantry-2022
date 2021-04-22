#ifndef __WT61C_TASK_H
#define __WT61C_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

typedef struct
{
	struct
	{
		float x;
		float y;
		float z;
	}acceleration; //加速度
	
	struct
	{
		float roll_x;
		float pitch_y;
		float yaw_z;
	}angle;  //角度
	
	float temperature;
	
}Wt61c_Data_t;

extern TaskHandle_t Wt61cTask_Handler;

void Wt61c_Task(void *pvParameters);

void Wt61c_Data_Update(void);

const Wt61c_Data_t* Get_Wt61c_Data(void);

#endif
