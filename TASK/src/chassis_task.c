#include "chassis_task.h"

#include "infantry.h"
#include "remoter_task.h"
#include "can1_motor.h"
#include "can2_motor.h"
#include "remoter.h"
#include "math2.h"
#include "judge_system.h"
#include "detect_task.h"

#define CHASSIS_SPEED_ZERO  0
#define OUTPUT_LIMIT(data, limit)  Float_Constrain(data, -limit, limit)
#define CHASSIS_MOTOR_DEFAULT_BASE_RATE 6.0f
#define CHASSIS_MOTOR_GYRO_BASE_RATE 5.0f

//相关变量定义
TaskHandle_t ChassisTask_Handler;

const static Rc_ctrl_t* remoter_control;
const static Robot_mode_t* chassis_robot_mode;
static const Motor_measure_t* yaw_motor;
const static Judge_data_t* judge_data;

static float chassis_motor_boost_rate = 1.0f;  //调用相应函数更改
static Pid_Position_t chassis_follow_pid = NEW_POSITION_PID(0.26, 0, 0.8, 5000, 500, 0, 1000, 500);  //底盘跟随PID

//函数声明
static uint16_t Calc_Gyro_Speed_By_Power_Limit(uint16_t power_limit);
static float Calc_Chassis_Follow(void);
static void Calc_Gyro_Motors_Speed(float* motors_speed, float rotate_speed, float move_direction, float x_move_speed, float y_move_speed);

void Chassis_Task(void *pvParameters)
{
	float motor_speed[4] = {0, 0, 0, 0};
	float follow_pid_output;
	
	yaw_motor = Get_Gimbal_Motor();  //底盘跟随时使用
	remoter_control = Get_Remote_Control_Point();  //遥控器数据
	chassis_robot_mode = Get_Robot_Mode_Point();  //机器人模式数据
	judge_data = Get_Judge_Data();
	
	vTaskDelay(200);
	
	while(1)
	{
		
		//键鼠控制
		if(chassis_robot_mode->control_device == 1)
		{
			
			switch(chassis_robot_mode->mouse_key_chassis_mode)
			{
				//底盘跟随
				case 1:
				{
					follow_pid_output = Calc_Chassis_Follow();

					motor_speed[0] = remoter_control->virtual_rocker.ch2 + remoter_control->virtual_rocker.ch3 + follow_pid_output + remoter_control->mouse.x/20;
					motor_speed[1] = remoter_control->virtual_rocker.ch2 - remoter_control->virtual_rocker.ch3 + follow_pid_output + remoter_control->mouse.x/20;
					motor_speed[2] = -remoter_control->virtual_rocker.ch2 + remoter_control->virtual_rocker.ch3 + follow_pid_output + remoter_control->mouse.x/20;
					motor_speed[3] = -remoter_control->virtual_rocker.ch2 - remoter_control->virtual_rocker.ch3 + follow_pid_output + remoter_control->mouse.x/20;
					
					motor_speed[0] *= (float)(CHASSIS_MOTOR_DEFAULT_BASE_RATE * chassis_motor_boost_rate);
					motor_speed[1] *= (float)(CHASSIS_MOTOR_DEFAULT_BASE_RATE * chassis_motor_boost_rate);
					motor_speed[2] *= (float)(CHASSIS_MOTOR_DEFAULT_BASE_RATE * chassis_motor_boost_rate);
					motor_speed[3] *= (float)(CHASSIS_MOTOR_DEFAULT_BASE_RATE * chassis_motor_boost_rate);

					break;
				}

				//小陀螺
				case 2:
				{
					Calc_Gyro_Motors_Speed(motor_speed, \
					Calc_Gyro_Speed_By_Power_Limit(judge_data->game_robot_status.chassis_power_limit), \
					GM6020_YAW_Angle_To_360(yaw_motor->mechanical_angle), \
					(float)remoter_control->virtual_rocker.ch3 * CHASSIS_MOTOR_GYRO_BASE_RATE * chassis_motor_boost_rate, \
					(float)remoter_control->virtual_rocker.ch2 * CHASSIS_MOTOR_GYRO_BASE_RATE * chassis_motor_boost_rate );

					break;
				}

				//特殊（云台陀螺仪失效时使用）
				case 3:
				{
					Calc_Gyro_Motors_Speed(motor_speed, \
					0, \
					GM6020_YAW_Angle_To_360(yaw_motor->mechanical_angle), \
					remoter_control->virtual_rocker.ch3 * 6, \
					remoter_control->virtual_rocker.ch2 * 5);
					
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
				case 3:
				{
					follow_pid_output = Calc_Chassis_Follow();
					
					motor_speed[0] = remoter_control->rc.ch2 + remoter_control->rc.ch3 + follow_pid_output + remoter_control->rc.ch0/2.9f;
					motor_speed[1] = remoter_control->rc.ch2 - remoter_control->rc.ch3 + follow_pid_output + remoter_control->rc.ch0/2.9f;
					motor_speed[2] = -remoter_control->rc.ch2 + remoter_control->rc.ch3 + follow_pid_output + remoter_control->rc.ch0/2.9f;
					motor_speed[3] = -remoter_control->rc.ch2 - remoter_control->rc.ch3 + follow_pid_output + remoter_control->rc.ch0/2.9f;
					
					motor_speed[0] *= 13;
					motor_speed[1] *= 13;
					motor_speed[2] *= 13;
					motor_speed[3] *= 13;
					
					break;
				}
				
				//底盘小陀螺
				case 2:
				case 4:
				{
					Calc_Gyro_Motors_Speed(motor_speed, \
					Calc_Gyro_Speed_By_Power_Limit(judge_data->game_robot_status.chassis_power_limit), \
					GM6020_YAW_Angle_To_360(yaw_motor->mechanical_angle), \
					remoter_control->rc.ch3 * 6, \
					remoter_control->rc.ch2 * 6);
					break;
				}
				
				//特殊
				case 5:
				{
					Calc_Gyro_Motors_Speed(motor_speed, \
					0, \
					GM6020_YAW_Angle_To_360(yaw_motor->mechanical_angle), \
					remoter_control->rc.ch3 * 10, \
					remoter_control->rc.ch2 * 10);
					break;
				}
				
			}
			
		}
		
		//数据大小限制
		//额定转速 469rpm
		//减速箱减速比约为19:1
		OUTPUT_LIMIT(&motor_speed[0], 8888);
		OUTPUT_LIMIT(&motor_speed[1], 8888);
		OUTPUT_LIMIT(&motor_speed[2], 8888);
		OUTPUT_LIMIT(&motor_speed[3], 8888);
		
		#if CHASSIS_SPEED_ZERO
			motor_speed[0] = 0;
			motor_speed[1] = 0;
			motor_speed[2] = 0;
			motor_speed[3] = 0;
		#endif
		
		//底盘电机速度设置
		Set_Chassis_Motors_Speed(motor_speed[0], motor_speed[1], motor_speed[2], motor_speed[3]);
		
    	vTaskDelay(5); //延时5ms，也就是1000个时钟节拍	
		
	}
	
	//vTaskDelete(NULL);
}

//半秒加速，mode为1或-1，max_rate>1
void Change_Chassis_Motor_Boost_Rate(int8_t mode, float max_rate)
{
	if(mode == 0)
	{
		chassis_motor_boost_rate = 1.0f;
		return;
	}
	chassis_motor_boost_rate += (max_rate-1.0f) / 35 * ((float)mode);
	Float_Constrain(&chassis_motor_boost_rate, 1.0f, max_rate);
}

//通过功率计算小陀螺时各个电机（M3508）的速度
static uint16_t Calc_Gyro_Speed_By_Power_Limit(uint16_t power_limit)
{
	if(power_limit < 30)
	{
		return 1800;
	}
	else if(power_limit > 300)
	{
		return 2000;
	}
	else if(power_limit > 100)
	{
		return 6000;
	}

	return power_limit*60;
}

/*
 * 底盘跟随PID计算
 * YAW_INIT_ANGLE宏定义是将云台的头和底盘的头对其，读出的GM6020机械角度
 * 依赖局部变量chassis_follow_pid，YAW_INIT_ANGLE宏定义数值，yaw轴云台当前机械角值
*/
#define YAW_INIT_ANGLE YAW_GM6020_HEAD_ANGLE
static float Calc_Chassis_Follow(void)
{
	float follow_tar = YAW_INIT_ANGLE;
	float follow_cur = yaw_motor->mechanical_angle;
	
	Handle_Angle8191_PID_Over_Zero(&follow_tar, &follow_cur);
	
	return Pid_Position_Calc(&chassis_follow_pid, follow_tar, follow_cur);	
}


/* 小陀螺运动 */
/**
  * @brief          计算小陀螺时，各个电机速度
  * 
  * 效果：底盘以设置速度小陀螺旋转，同时可以以云台枪管为头，前进后退、左右平移
  * 
  * @author         Bashpow
  * @param[in]      储存电机速度的指针
  * @param[in]      旋转的电机速度
  * @param[in]      运动的方向角度0°~360°，以机器朝向为0°(云台GM6020电机相对底盘的角度)
  * @param[in]      x轴速度（X摇杆）
  * @param[in]      y轴速度（Y摇杆）
  * @retval         返回空
  */
static void Calc_Gyro_Motors_Speed(float* motors_speed, float rotate_speed, float move_direction, float x_move_speed, float y_move_speed)
{
	//判断角度是否错误
	if(move_direction > 360)
	{
		return;
	}
	
	//角度换算弧度
	float move_radin = move_direction * 3.14159f / 180.0f;
	float radin_sin = arm_sin_f32(move_radin);
	float radin_cos = arm_cos_f32(move_radin);
	
	/* 陀螺速度赋值 */
	motors_speed[0] = rotate_speed;
	motors_speed[1] = rotate_speed;
	motors_speed[2] = rotate_speed;
	motors_speed[3] = rotate_speed;
	
	//计算速度增加值
	float x_x_speed = x_move_speed*radin_cos;
	float x_y_speed = x_move_speed*radin_sin;
	
	float y_x_speed = y_move_speed*radin_sin;
	float y_y_speed = y_move_speed*radin_cos;
	
	motors_speed[0] += ( (x_x_speed - x_y_speed) + (y_x_speed + y_y_speed) );
	motors_speed[1] += ( (-x_x_speed - x_y_speed) + (-y_x_speed + y_y_speed) );
	motors_speed[2] += ( (x_x_speed + x_y_speed) + (y_x_speed - y_y_speed) );
	motors_speed[3] += ( (-x_x_speed + x_y_speed) + (-y_x_speed - y_y_speed) );
	
}


