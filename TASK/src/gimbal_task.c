#include "gimbal_task.h"

#include "math2.h"
#include "remoter.h"
#include "remoter_task.h"
#include "can2_motor.h"
#include "wt61c_task.h"
#include "autoaim.h"

#define CALC_AUTOAIM_YAW_PID()    Pid_Increment_Calc(&autoaim_yaw_pid, autoaim_data->x_yaw, AUTOAIM_X_YAW_CUR);
#define CALC_AUTOAIM_PITCH_PID()  Pid_Increment_Calc(&autoaim_pitch_pid, autoaim_data->y_pitch, AUTOAIM_Y_PITCH_CUR);

//变量定义
TaskHandle_t GimbalTask_Handler;
const static Rc_ctrl_t* remoter_control;
const static Robot_mode_t* chassis_robot_mode;
const static Wt61c_Data_t* wt61c_data;
const static Auto_aim_t* autoaim_data;

static float yaw_angle_set = 0;
static float pitch_angle_set = PITCH_MID_ANGLE;

static Pid_Increment_t autoaim_yaw_pid = NEW_INCREMENT_PID(0.022, 0.0025, 0, 1.0);
static Pid_Increment_t  autoaim_pitch_pid = NEW_INCREMENT_PID(0.085, 0.07, 0.01, 1000);

void Gimbal_Task(void *pvParameters)
{
	float yaw_speed, pitch_speed;  //云台电机速度
	wt61c_data = Get_Wt61c_Data(); //陀螺仪数据
	remoter_control = Get_Remote_Control_Point();  //遥控器数据
	chassis_robot_mode = Get_Robot_Mode_Point();  //机器人模式数据
	autoaim_data = Get_Auto_Aim_Msg();  //自瞄数据

	vTaskDelay(200);
	
	while(1)
	{
		
		//键鼠控制
		if(chassis_robot_mode->control_device == 1)
		{
			switch(chassis_robot_mode->mouse_key_gimbal_mode)
			{
				//手动模式
				case 1:
				{
					yaw_angle_set -= (remoter_control->mouse.x) / 58.0f;
					pitch_angle_set += (remoter_control->mouse.y) / 1.6f;

					//yaw角度回环
					if(yaw_angle_set>360) yaw_angle_set -= 360;
					if(yaw_angle_set<0) yaw_angle_set += 360;
					
					/* Pitach角度限制 */
					Float_Constrain(&pitch_angle_set, PITCH_UP_LIMIT, PITCH_DOWN_LIMIT);

					yaw_speed = Calc_Yaw_Angle360_Pid(yaw_angle_set, wt61c_data->angle.yaw_z);
					pitch_speed = Calc_Pitch_Angle8191_Pid(pitch_angle_set);
					
					break;
					
				}

				//自瞄模式
				case 2:
				{
					//找到目标，自瞄介入
					if(autoaim_data->identifie_ready)
					{
						yaw_angle_set -= CALC_AUTOAIM_YAW_PID();
						pitch_angle_set -= CALC_AUTOAIM_PITCH_PID();
					}
					
					yaw_angle_set -= (remoter_control->mouse.x) / 58.0f;
					pitch_angle_set += (remoter_control->mouse.y) / 1.6f;

					//yaw角度回环
					if(yaw_angle_set>360) yaw_angle_set -= 360;
					if(yaw_angle_set<0) yaw_angle_set += 360;
					
					/* Pitach角度限制 */
					Float_Constrain(&pitch_angle_set, PITCH_UP_LIMIT, PITCH_DOWN_LIMIT);

					yaw_speed = Calc_Yaw_Angle360_Pid(yaw_angle_set,wt61c_data->angle.yaw_z);
					pitch_speed = Calc_Pitch_Angle8191_Pid(pitch_angle_set);

					break;
				}

				//特殊
				case 3:
				{
					pitch_angle_set += remoter_control->mouse.y / 3.0f;

					/* Pitach角度限制 */
					Float_Constrain(&pitch_angle_set, PITCH_UP_LIMIT, PITCH_DOWN_LIMIT);

					yaw_speed = (- remoter_control->mouse.x) / 1.6f;
					pitch_speed = Calc_Pitch_Angle8191_Pid(pitch_angle_set);

					break;
				}

			}
		}
		
		
		//遥控器模式
		else if(chassis_robot_mode->control_device == 2)
		{
			switch(chassis_robot_mode->rc_motion_mode)
			{
				//底盘跟随
				case 1:
				//2底盘小陀螺云台自由运动
				case 2:
				{
					yaw_angle_set -= (remoter_control->rc.ch0) / 160.0f;
					pitch_angle_set -= (remoter_control->rc.ch1) / 12.0f;
					
					//yaw角度回环
					if(yaw_angle_set>360) yaw_angle_set -= 360;
					if(yaw_angle_set<0) yaw_angle_set += 360;
					
					/* Pitach角度限制 */
					Float_Constrain(&pitch_angle_set, PITCH_UP_LIMIT, PITCH_DOWN_LIMIT);

					yaw_speed = Calc_Yaw_Angle360_Pid(yaw_angle_set, wt61c_data->angle.yaw_z);
					pitch_speed = Calc_Pitch_Angle8191_Pid(pitch_angle_set);

					break;
				}
				
				//自瞄模式
				case 3:
				case 4:
				{
					//找到目标，自瞄介入
					if(autoaim_data->identifie_ready)
					{
						yaw_angle_set -= CALC_AUTOAIM_YAW_PID();
						pitch_angle_set -= CALC_AUTOAIM_PITCH_PID();
					}

					//手动控制
					yaw_angle_set -= (remoter_control->rc.ch0) / 160.0f;
					pitch_angle_set -= (remoter_control->rc.ch1) / 12.0f;

					//yaw角度回环
					if(yaw_angle_set>360) yaw_angle_set -= 360;
					if(yaw_angle_set<0) yaw_angle_set += 360;
					
					/* Pitach角度限制 */
					Float_Constrain(&pitch_angle_set, PITCH_UP_LIMIT, PITCH_DOWN_LIMIT);

					yaw_speed = Calc_Yaw_Angle360_Pid(yaw_angle_set, wt61c_data->angle.yaw_z);
					pitch_speed = Calc_Pitch_Angle8191_Pid(pitch_angle_set);

					break;
				}

				//特殊
				case 5:
				{
					pitch_angle_set -= remoter_control->rc.ch1 / 12.0f;

					/* Pitach角度限制 */
					Float_Constrain(&pitch_angle_set, PITCH_UP_LIMIT, PITCH_DOWN_LIMIT);

					yaw_speed = (- remoter_control->rc.ch0) / 5.5f;
					pitch_speed = Calc_Pitch_Angle8191_Pid(pitch_angle_set);

					break;
				}
				
			}
			
		}
		
		Float_Constrain(&yaw_speed, -125, 125);  //yaw轴速度限制
		Set_Gimbal_Motors_Speed(yaw_speed, pitch_speed);

		vTaskDelay(12);

	}
	
	//vTaskDelete(NULL);
	
}

