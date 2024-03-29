#include "settings_task.h"
#include "judge_system.h"
#include "detect_task.h"
#include "can1_motor.h"
#include "autoaim.h"

TaskHandle_t SettingsTask_Handler;

void Settings_Task(void *pvParameters)
{
	vTaskDelay(1000);
	
	for(;;)
	{
		// 底盘功率设置
        if(Get_Module_Online_State(SUPER_CAPACITOR) == 1u)
		{
			if(Get_Module_Online_State(JUDGE_SYSTEM) == 1u)
			{
				if((Get_Judge_Data()->ext_game_robot_status_t.chassis_power_limit - 2) != ((uint16_t)(Get_Super_Capacitor()->target_power)))
				{
					Set_Super_Capacitor( (Get_Judge_Data()->ext_game_robot_status_t.chassis_power_limit-2) * 100);
				}
			}
			else
			{
				Set_Super_Capacitor(4500);
			}
		}

        // 自瞄设置
        if((Get_Module_Online_State(JUDGE_SYSTEM) == 1u) && (Get_Module_Online_State(AUTO_AIM) == 1u))
		{
			if(Get_Judge_Data()->ext_game_robot_status_t.robot_id<10)
			{
				if(Get_Auto_Aim_Msg()->target_color != 0x0F0F)
				{
					Set_Target_Color(0x0F0F);
				}
			}
			else
			{
				if(Get_Auto_Aim_Msg()->target_color != 0x1F1F)
				{
					Set_Target_Color(0x1F1F);
				}
			}
		}

		vTaskDelay(500);
	}
	
	
	//vTaskDelete(NULL);
	
}
