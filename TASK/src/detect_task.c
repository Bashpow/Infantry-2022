#include "detect_task.h"

#include "listen.h"
#include "led.h"
#include "usart3.h"
#include "can1_motor.h"
#include "judge_system.h"

#define MODULE_REONLINE(index) (module_status[index].time_out_flag==0 && module_status[index].old_time_out_flag==1)
#define MODULE_OFFLINE(index)  (module_status[index].time_out_flag==1 && module_status[index].old_time_out_flag==0)

enum errorList
{
	remote_control = 0u, //遥控器
	chassis_motor,       //底盘电机
	gimbal_motor,        //云台电机
	shooter_motor,       //发射机构电机
	auto_aim,            //自瞄NUC
	judge_system,        //裁判系统
	super_capacitor,     //超级电容
	gyroscope            //陀螺仪
};

TaskHandle_t DetectTask_Handler;
static Module_status_t module_status[8];
static const Judge_data_t *judge_data;
static const Super_capacitor_t *super_capacitor_data;

static void Detect_Task_Init(void);

void Detect_Task(void *pvParameters)
{
	Detect_Task_Init();

	u8 cap_send_cnt = 5;
	judge_data = Get_Judge_Data();
	super_capacitor_data = Get_Super_Capacitor();

	vTaskDelay(800);
	
	while(1)
	{

		for(u8 i=0; i<8; i++)
		{
			//各个模块状态刷新
			Module_Status_Listen(&module_status[i]);
			
			if(MODULE_REONLINE(i))
			{
				LED_FLOW_ON(i);
				INFO_LOG("Module%d Online.\r\n\r\n", i);
			}
			else if(MODULE_OFFLINE(i))
			{
				LED_FLOW_OFF(i);
				INFO_LOG("Module%d Offline.\r\n\r\n", i);
			}
			
		}
		
		//底盘功率设置
		if(cap_send_cnt > 2)
		{
			/* 功率限制 */
			if(module_status[judge_system].time_out_flag==0 && module_status[super_capacitor].time_out_flag==0)  //判断裁判系统、底盘是否同时上线
			{
				//判断超级电容目标功率与裁判系统限制功率-2是否相符，否设置超级电容
				if((judge_data->game_robot_status.chassis_power_limit - 2) != ((uint16_t)(super_capacitor_data->target_power)))
				{
					Set_Super_Capacitor( (judge_data->game_robot_status.chassis_power_limit-2) * 100);
					cap_send_cnt = 0;
				}
			}
		}
		else
		{
			cap_send_cnt++;
		}
		
		LED_GREEN_TOGGLE;

		vTaskDelay(250);
	}
	
	
	//vTaskDelete(NULL);
	
}

static void Detect_Task_Init(void)
{
	for(u8 i=0; i<8; i++)
	{
		Module_Status_Init(&module_status[i], 5, NULL, NULL);
	}
	
	module_status[5].reload_cnt = 10;
	
}

u8 Get_Module_Online_State(u8 id)
{
	// return (!module_status[id].time_out_flag);
	if(module_status[id].time_out_flag == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void Detect_Reload(u8 index)
{
	Module_Status_Reload(&module_status[index]);
}

void Classis_Reload(int8_t motor_index)
{
	static u8 chassis_motor[4] = {0, 0, 0, 0};
	
	if(motor_index>=0 && motor_index<=3)
	{
		chassis_motor[motor_index] = 1;
	}
	else
	{
		return;
	}
	if(chassis_motor[0] && chassis_motor[1] && chassis_motor[2] && chassis_motor[3])
	{
		chassis_motor[0] = 0;
		chassis_motor[1] = 0;
		chassis_motor[2] = 0;
		chassis_motor[3] = 0;
		Module_Status_Reload(&module_status[1]);
	}
}

void Gimbal_Reload(int8_t motor_index)
{
	static u8 gimbal_motor[2] = {0, 0};
	
	if(motor_index>=0 && motor_index<=1)
	{
		gimbal_motor[motor_index] = 1;
	}
	else
	{
		return;
	}
	if(gimbal_motor[0] && gimbal_motor[1])
	{
		gimbal_motor[0] = 0;
		gimbal_motor[1] = 0;
		Module_Status_Reload(&module_status[2]);
	}
}

void Shooter_Reload(void)
{
	Module_Status_Reload(&module_status[3]);
}
