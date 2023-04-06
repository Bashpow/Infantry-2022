#include "detect_task.h"
#include "listen.h"
#include "led.h"
#include "usart3.h"

#define MODULE_REONLINE(index) (module_status[index].time_out_flag==0 && module_status[index].old_time_out_flag==1)
#define MODULE_OFFLINE(index)  (module_status[index].time_out_flag==1 && module_status[index].old_time_out_flag==0)

static void Detect_Task_Init(void);

// 分别代表8个模块
static Module_status_t module_status[8];

TaskHandle_t DetectTask_Handler;

void Detect_Task(void *pvParameters)
{
	Detect_Task_Init();
	vTaskDelay(800);
	
	for(;;)
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
		
		LED_GREEN_TOGGLE;
		vTaskDelay(250);
	}
	//vTaskDelete(NULL);
}

/**
 * @brief 各个模块初始化
 * 
 */
static void Detect_Task_Init(void)
{
	for(uint8_t i=0; i<8; i++)
	{
		Module_Status_Init(&module_status[i], 5, NULL, NULL);
	}
	module_status[5].reload_cnt = 10;
}

/**
 * @brief 获取模块在线状态
 * 
 * @param id 模块id
 * @return uint8_t 模块在线---1u，模块离线---0u
 */
uint8_t Get_Module_Online_State(uint8_t id)
{
	return Get_Module_State(&module_status[id]);
}

/**
 * @brief 刷新模块
 * 
 * @param id 模块id
 */
void Detect_Reload(uint8_t id)
{
	Module_Status_Reload(&module_status[id]);
}


/**
 * @brief 刷新底盘电机状态
 * 
 * @param motor_index must be 0~3,分别对应底盘四个电机
 */
void Classis_Reload(const int8_t motor_index)
{
	// bit0-3对应4个电机
	static uint8_t chassis_motor_state = 0u;

	// 刷新对应bit
	if (motor_index >= 0 && motor_index <= 3)
	{
		chassis_motor_state |= (1u << motor_index);
	}
	else
	{
		return;
	}
	// 判断四轮是否都上线，若上线则刷新状态
	if ((chassis_motor_state & 0x0F) == 0x0F)
	{
		chassis_motor_state = 0;
		Module_Status_Reload(&module_status[CHASSIS_MOTOR]);
	}
}

/**
 * @brief 刷新云台电机状态
 * 
 * @param motor_index must be 0~1,分别对应底盘两个电机
 */
void Gimbal_Reload(int8_t motor_index)
{
	static uint8_t gimbal_motor[2] = {0, 0};
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
		Module_Status_Reload(&module_status[GIMBAL_MOTOR]);
	}
}

/**
 * @brief 刷新发射机构3电机状态
 * 
 * @param motor_index must be 0~2,分别对应3个电机
 */
void Shooter_Reload(const int8_t motor_index)
{
	static uint8_t shooter_motor_state = 0u; // bit0-3对应4个电机
	if (motor_index >= 0 && motor_index <= 2) {
		shooter_motor_state |= (1u << motor_index); // 刷新对应bit
	} else {
		return;
	}	
	if ((shooter_motor_state & 0x07) == 0x07) { // 判断四轮是否都上线，若上线则刷新状态
		shooter_motor_state = 0;
		Module_Status_Reload(&module_status[SHOOTER_MOTOR]);
	}
}
