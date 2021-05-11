#ifndef __REMOTER_TASK_H
#define __REMOTER_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "remoter.h"

typedef struct
{
	u8 control_device; //控制设备 1键鼠模式 2遥控器模式
	u8 rc_motion_mode; //底盘云台模式 1底盘跟随 2小陀螺 3自瞄跟随 4自瞄小陀螺 5特殊
	u8 mouse_key_chassis_mode; //1跟随 2小陀螺 3特殊
	u8 mouse_key_gimbal_mode; //1手动 2自瞄 3特殊
	u8 shoot_mode; //射击模式 1单发 2三连发 3连发速度一 4连发速度二
	u8 fric_cover_mode; //摩擦轮模式 0关闭 1慢速 2快速 3开舱盖
	
}Robot_mode_t;



extern TaskHandle_t RemoterTask_Handler;

void Remoter_Task(void *pvParameters);
void Rc_Data_Update(void);
void Fric_Reset(void);

const Rc_ctrl_t *Get_Remote_Control_Point(void);
const Robot_mode_t *Get_Robot_Mode_Point(void);


#endif

