#include "can2_motor.h"

#include "can2.h"
#include "pid.h"
#include "math2.h"
#include "detect_task.h"
 

/* 定义变量 */
static Motor_measure_t shooter_wave_motor; //波轮电机数据
static Motor_measure_t gimbal_motor[2]; //云台电机数据

static Pid_Position_t motor_wave_speed_pid = NEW_POSITION_PID(4.2, 0, 0.1, 2000, 10000, 0, 1000, 500); //波轮速度PID

static Pid_Position_t motor_yaw_speed_pid = NEW_POSITION_PID(600, 110, 74, 5000, 30000, 0, 1000, 500); //yaw电机速度PID
static Pid_Position_t motor_yaw_angle_pid = NEW_POSITION_PID(0.6, 0, 15, 220, 300, 0, 3000, 500); //yaw电机角度PID

static Pid_Position_t motor_pitch_speed_pid = NEW_POSITION_PID(120, 2, 0, 220, 30000, 0, 1000, 500); //pitch电机速度PID
static Pid_Position_t motor_pitch_angle_pid = NEW_POSITION_PID(0.8, 0, 0.1, 100, 300, 0, 3000, 500); //pitch电机角度PID

/* 函数声明 */
static void Can2_Hook(CanRxMsg *rx_message);

/* CAN1 send and receive ID wheel_left */
typedef enum
{
	CAN_SHOOTER_ALL_ID = 0x200,
	CAN_3508_WAVE_ID = 0x201,

	CAN_GIMBAL_ALL_ID = 0x2FF,
	CAN_YAW_MOTOR_ID = 0x209,  //x
	CAN_PITCH_MOTOR_ID = 0x20A,  //y
	
} can1_msg_id_e;


void CAN2_RX0_IRQHandler(void)
{
	
	static CanRxMsg can2_rx_msg;
	if (CAN_GetITStatus(CAN2,CAN_IT_FMP0)!= RESET)
	{ 
		CAN_ClearITPendingBit(CAN2, CAN_IT_FF0);
		CAN_Receive(CAN2, CAN_FIFO0, &can2_rx_msg);
		Can2_Hook(&can2_rx_msg);
	}
	
}

//计算发射机构波轮电机速度PID，并输出
void Set_Shooter_Wave_Motors_Speed(float wave_wheel)
{
	Can2_Send_4Msg(CAN_SHOOTER_ALL_ID,\
	               Pid_Position_Calc(&motor_wave_speed_pid, wave_wheel, shooter_wave_motor.speed_rpm),\
	               0,\
	               0,\
	               0);
}

//获取云台2个电机数据指针
const Motor_measure_t *Get_Gimbal_Motor(void)
{
    return gimbal_motor;
}

//计算速度PID，并输出给电机
void Set_Gimbal_Motors_Speed(float speed_yaw, float speed_pitch)
{
	static uint8_t fre=0;
	float aaa = Pid_Position_Calc(&motor_yaw_speed_pid, speed_yaw, gimbal_motor[0].speed_rpm);
	Can2_Send_4Msg(CAN_GIMBAL_ALL_ID,\
	               aaa,\
	               Pid_Position_Calc(&motor_pitch_speed_pid, speed_pitch, gimbal_motor[1].speed_rpm),\
	               0,\
				   0);
	//printf("%.1f,%d\r\n", motor_pitch_speed_pid.output,(gimbal_motor+1)->mechanical_angle);
	fre++;
	//if(fre == 100)
	{
	 // fre = 0;	
	  printf("%.1f, %d, %.1f, %.1f, %.1f, %.1f\r\n",speed_yaw, gimbal_motor->speed_rpm, motor_yaw_speed_pid.p_out,motor_yaw_speed_pid.i_out,motor_yaw_speed_pid.d_out,motor_yaw_speed_pid.output);
	}
}

//计算YAW轴PID，角度格式为0~360
float Calc_Yaw_Angle360_Pid(float tar_angle, float cur_speed)
{
	float yaw_tar_angle = tar_angle;
	float yaw_cur_angle = cur_speed;
	
	Handle_Angle360_PID_Over_Zero(&yaw_tar_angle, &yaw_cur_angle);
	return Pid_Position_Calc(&motor_yaw_angle_pid, yaw_tar_angle, yaw_cur_angle);
}

//计算Pitch轴PID，角度格式为0~8191
float Calc_Pitch_Angle8191_Pid(float tar_angle)
{
	float pitch_tar_angle = tar_angle;
	float pitch_cur_angle = gimbal_motor[1].mechanical_angle;
	Pitch_Angle_Limit(&pitch_tar_angle,PITCH_DOWN_LIMIT,PITCH_UP_LIMIT);
	Handle_Angle8191_PID_Over_Zero(&pitch_tar_angle,&pitch_cur_angle);
	return Pid_Position_Calc(&motor_pitch_angle_pid, pitch_tar_angle, pitch_cur_angle);	
}


//统一处理can中断函数，并且记录发送数据的时间，作为离线判断依据
static void Can2_Hook(CanRxMsg *rx_message)
{
	switch (rx_message->StdId)
	{
		case CAN_3508_WAVE_ID:
		{
			//处理电机数据宏函数
			Calculate_Motor_Data(&shooter_wave_motor, rx_message);
			
			//记录时间
			Shooter_Reload();
			break;
		}
		
		case CAN_YAW_MOTOR_ID:
		case CAN_PITCH_MOTOR_ID:
		{
			uint8_t i = 0;
			//处理电机ID号
			i = rx_message->StdId - CAN_YAW_MOTOR_ID;
			//处理电机数据宏函数
			Calculate_Motor_Data(&gimbal_motor[i], rx_message);
			//记录时间
			Gimbal_Reload(i);
			break;
		}
		
		default:
		{
			break;
		}
	}
}


