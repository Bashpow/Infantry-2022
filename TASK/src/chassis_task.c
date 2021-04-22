#include "chassis_task.h"

#include "remoter_task.h"
#include "can1_motor.h"
#include "can2_motor.h"
#include "remoter.h"
#include "math2.h"
#include "judge_system.h"
#include "detect_task.h"

#define CHASSIS_SPEED_ZERO 0
#define OUTPUT_LIMIT(data, limit) Int16_Constrain(data, -limit, limit)

//��ر�������
TaskHandle_t ChassisTask_Handler;

const static Rc_ctrl_t* remoter_control;
const static Robot_mode_t* chassis_robot_mode;
static const Motor_measure_t* yaw_motor;
const static Judge_data_t* judge_data;
// const static Super_capacitor_t* super_capacitor;

static Pid_Position_t chassis_follow_pid = NEW_POSITION_PID(0.5, 0, 0.05, 2000, 8600, 0, 1000, 500);  //���̸���PID

static float chassis_motor_dynamic_rate = 10;

//��������
static uint16_t Calc_Gyro_Speed_By_Power_Limit(uint16_t power_limit);
static float Calc_Chassis_Follow(void);
static void Calc_Gyro_Motors_Speed(int16_t* motors_speed, int16_t rotate_speed, uint16_t move_direction, int16_t x_move_speed, int16_t y_move_speed);
// static void Change_Chassis_Motor_Dynamic_Rate(void);

void Chassis_Task(void *pvParameters)
{
	int16_t motor_speed[4] = {0, 0, 0, 0};
	float follow_pid_output;
	
	yaw_motor = Get_Gimbal_Motor();  //���̸���ʱʹ��
	remoter_control = Get_Remote_Control_Point();  //ң��������
	chassis_robot_mode = Get_Robot_Mode_Point();  //������ģʽ����
	judge_data = Get_Judge_Data();
	// super_capacitor = Get_Super_Capacitor();
	
	vTaskDelay(200);
	
	while(1)
	{
		
		//�������
		if(chassis_robot_mode->control_device == 1)
		{
			
			switch(chassis_robot_mode->mouse_key_chassis_mode)
			{
				//���̸���
				case 1:
				{
					follow_pid_output = Calc_Chassis_Follow();
					//Change_Chassis_Motor_Dynamic_Rate();

					motor_speed[0] = remoter_control->virtual_rocker.ch2 + remoter_control->virtual_rocker.ch3 + follow_pid_output + remoter_control->mouse.x/20;
					motor_speed[1] = remoter_control->virtual_rocker.ch2 - remoter_control->virtual_rocker.ch3 + follow_pid_output + remoter_control->mouse.x/20;
					motor_speed[2] = -remoter_control->virtual_rocker.ch2 + remoter_control->virtual_rocker.ch3 + follow_pid_output + remoter_control->mouse.x/20;
					motor_speed[3] = -remoter_control->virtual_rocker.ch2 - remoter_control->virtual_rocker.ch3 + follow_pid_output + remoter_control->mouse.x/20;
					
					motor_speed[0] *= chassis_motor_dynamic_rate;
					motor_speed[1] *= chassis_motor_dynamic_rate;
					motor_speed[2] *= chassis_motor_dynamic_rate;
					motor_speed[3] *= chassis_motor_dynamic_rate;

					break;
				}

				//С����
				case 2:
				{
					Calc_Gyro_Motors_Speed(motor_speed, \
					Calc_Gyro_Speed_By_Power_Limit(judge_data->game_robot_status.chassis_power_limit), \
					GM6020_YAW_Angle_To_360(yaw_motor->mechanical_angle), \
					remoter_control->virtual_rocker.ch3 * 5, \
					remoter_control->virtual_rocker.ch2 * 5 );
					break;
				}

				//���⣨��̨������ʧЧʱʹ�ã�
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
		
		
		//ң����ģʽ
		else if(chassis_robot_mode->control_device == 2)
		{
			switch(chassis_robot_mode->motion_mode)
			{
				//���̸���
				case 1:
				case 3:
				{
					follow_pid_output = Calc_Chassis_Follow();
					
					motor_speed[0] = remoter_control->rc.ch2 + remoter_control->rc.ch3 + follow_pid_output + remoter_control->rc.ch0/16.0f;
					motor_speed[1] = remoter_control->rc.ch2 - remoter_control->rc.ch3 + follow_pid_output + remoter_control->rc.ch0/16.0f;
					motor_speed[2] = -remoter_control->rc.ch2 + remoter_control->rc.ch3 + follow_pid_output + remoter_control->rc.ch0/16.0f;
					motor_speed[3] = -remoter_control->rc.ch2 - remoter_control->rc.ch3 + follow_pid_output + remoter_control->rc.ch0/16.0f;
					
					motor_speed[0] *= 11;
					motor_speed[1] *= 11;
					motor_speed[2] *= 11;
					motor_speed[3] *= 11;
					
					break;
				}
				
				//����С����
				case 2:
				case 4:
				{
					Calc_Gyro_Motors_Speed(motor_speed, \
					Calc_Gyro_Speed_By_Power_Limit(judge_data->game_robot_status.chassis_power_limit), \
					GM6020_YAW_Angle_To_360(yaw_motor->mechanical_angle), \
					remoter_control->rc.ch3 * 5, \
					remoter_control->rc.ch2 * 5);
					break;
				}
				
				//����
				case 5:
				{
					Calc_Gyro_Motors_Speed(motor_speed, \
					0, \
					GM6020_YAW_Angle_To_360(yaw_motor->mechanical_angle), \
					remoter_control->rc.ch3 * 9, \
					remoter_control->rc.ch2 * 9);

					break;
				}
				
			}
			
		}
		
		//���ݴ�С����
		OUTPUT_LIMIT(&motor_speed[0], 8500);
		OUTPUT_LIMIT(&motor_speed[1], 8500);
		OUTPUT_LIMIT(&motor_speed[2], 8500);
		OUTPUT_LIMIT(&motor_speed[3], 8500);
		
		#if CHASSIS_SPEED_ZERO
			motor_speed[0] = 0;
			motor_speed[1] = 0;
			motor_speed[2] = 0;
			motor_speed[3] = 0;
		#endif

		//���̵���ٶ�����
		Set_Chassis_Motors_Speed(motor_speed[0], motor_speed[1], motor_speed[2], motor_speed[3]);
		
    	vTaskDelay(5); //��ʱ5ms��Ҳ����1000��ʱ�ӽ���	
		
	}
	
	//vTaskDelete(NULL);
}

//ͨ�����ʼ���С����ʱ���������M3508�����ٶ�
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
 * ���̸���PID����
 * YAW_INIT_ANGLE�궨���ǽ���̨��ͷ�͵��̵�ͷ���䣬������GM6020��е�Ƕ�
 * �����ֲ�����chassis_follow_pid��YAW_INIT_ANGLE�궨����ֵ��yaw����̨��ǰ��е��ֵ
*/
#define YAW_INIT_ANGLE 2735
static float Calc_Chassis_Follow(void)
{
	float follow_tar = YAW_INIT_ANGLE;
	float follow_cur = yaw_motor->mechanical_angle;
	
	Handle_Angle8191_PID_Over_Zero(&follow_tar, &follow_cur);
	
	return Pid_Position_Calc(&chassis_follow_pid, follow_tar, follow_cur);	
}


/* С�����˶� */
/**
  * @brief          ����С����ʱ����������ٶ�
  * 
  * Ч���������������ٶ�С������ת��ͬʱ��������̨ǹ��Ϊͷ��ǰ�����ˡ�����ƽ��
  * 
  * @author         Bashpow
  * @param[in]      �������ٶȵ�ָ��
  * @param[in]      ��ת�ĵ���ٶ�
  * @param[in]      �˶��ķ���Ƕ�0��~360�㣬�Ի�������Ϊ0��(��̨GM6020�����Ե��̵ĽǶ�)
  * @param[in]      x���ٶȣ�Xҡ�ˣ�
  * @param[in]      y���ٶȣ�Yҡ�ˣ�
  * @retval         ���ؿ�
  */
static void Calc_Gyro_Motors_Speed(int16_t* motors_speed, int16_t rotate_speed, uint16_t move_direction, int16_t x_move_speed, int16_t y_move_speed)
{
	//�жϽǶ��Ƿ����
	if(move_direction > 360)
	{
		return;
	}
	
	//�ǶȻ��㻡��
	float move_radin = move_direction * 3.14159 /180;
	float radin_sin = arm_sin_f32(move_radin);
	float radin_cos = arm_cos_f32(move_radin);
	
	/* �����ٶȸ�ֵ */
	motors_speed[0] = rotate_speed;
	motors_speed[1] = rotate_speed;
	motors_speed[2] = rotate_speed;
	motors_speed[3] = rotate_speed;
	
	//�����ٶ�����ֵ
	float x_x_speed = x_move_speed*radin_cos;
	float x_y_speed = x_move_speed*radin_sin;
	
	float y_x_speed = y_move_speed*radin_sin;
	float y_y_speed = y_move_speed*radin_cos;
	
	motors_speed[0] += ( (x_x_speed - x_y_speed) + (y_x_speed + y_y_speed) );
	motors_speed[1] += ( (-x_x_speed - x_y_speed) + (-y_x_speed + y_y_speed) );
	motors_speed[2] += ( (x_x_speed + x_y_speed) + (y_x_speed - y_y_speed) );
	motors_speed[3] += ( (-x_x_speed + x_y_speed) + (-y_x_speed - y_y_speed) );
	
}

/*
static void Change_Chassis_Motor_Dynamic_Rate(void)
{
	if(Get_Module_Online_State(6)) //��������
	{
		if(super_capacitor->cap_voltage > 20.0f)
		{
			chassis_motor_dynamic_rate = 13;
			return;
		}

		else if(super_capacitor->cap_voltage < 20.0f && super_capacitor->cap_voltage > 12.0f)
		{
			//chassis_motor_dynamic_rate = 13.0f - (20.0f - super_capacitor->cap_voltage)*0.5f; //�ɻ���
			chassis_motor_dynamic_rate = 3.0f + super_capacitor->cap_voltage*0.5f; //�ɻ���
			return;
		}

		else
		{
			chassis_motor_dynamic_rate = 8.8f;
		}

	}
	else
	{
		chassis_motor_dynamic_rate = 11;
	}
}
*/
