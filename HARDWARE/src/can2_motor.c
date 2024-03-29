#include "can2_motor.h"

#include "can2.h"
#include "pid.h"
#include "math2.h"
#include "detect_task.h"
#include "cansend_task.h"

static Motor_measure_t friction_wheel_up_motor; // ??????????????????????????????????????
static Motor_measure_t friction_wheel_down_motor; // ??????????????????????????????????????
static Pid_Position_t friction_wheel_up_motor_speed_pid = NEW_POSITION_PID(10.8, 0.8, 0.2, 2000, 10000, 0, 1000, 500); //????????????????????PID
static Pid_Position_t friction_wheel_down_motor_speed_pid = NEW_POSITION_PID(10.8, 0.8, 0.2, 2000, 10000, 0, 1000, 500); //????????????????????PID

/* ????????????????????? */
static Motor_measure_t shooter_wave_motor; //?????????????????????????????
static Motor_measure_t gimbal_motor[2]; //???????��?????????????????????

static Pid_Position_t motor_wave_speed_pid = NEW_POSITION_PID(10.8, 0.8, 0.2, 2000, 10000, 0, 1000, 500); //????????????????????PID

static Pid_Position_t motor_yaw_speed_pid = NEW_POSITION_PID(1800, 0.8, 0.2, 5000, 30000, 0, 1000, 500); //yaw?????????????????PID
static Pid_Position_t motor_yaw_angle_pid = NEW_POSITION_PID(2.4, 0.01, 1.8, 5, 125, 0, 3000, 500); //yaw?????????????????PID

Pid_Position_t motor_pitch_speed_pid = NEW_POSITION_PID(280, 30, 0.5, 220, 30000, 0, 1000, 500); //pitch?????????????????PID
Pid_Position_t motor_pitch_angle_pid = NEW_POSITION_PID(0.18, 0.01, 0.002, 100, 300, 0, 3000, 500); //pitch?????????????????PID

/* ???????????????????????? */
static void Can2_Hook(CanRxMsg *rx_message);
static CanRxMsg can2_rx_msg;
CanRxMsg *GetCan2_RXD_Msg(void)
{
	return &can2_rx_msg;
}
/* CAN1 send and receive ID wheel_left */
typedef enum
{
	CAN_SHOOTER_ALL_ID = 0x200,
	CAN_3508_WAVE_ID = 0x201,

	CAN_3508_FRICTION_WHEEL_UP_ID = 0x203,
	CAN_3508_FRICTION_WHEEL_DOWN_ID = 0x204,

	CAN_GIMBAL_ALL_ID = 0x2FF,
	CAN_YAW_MOTOR_ID = 0x20B,  //x
	CAN_PITCH_MOTOR_ID = 0x209,  //y
	
} can1_msg_id_e;


void CAN2_RX0_IRQHandler(void)
{

	if (CAN_GetITStatus(CAN2,CAN_IT_FMP0)!= RESET)
	{ 
		CAN_ClearITPendingBit(CAN2, CAN_IT_FF0);
		CAN_Receive(CAN2, CAN_FIFO0, &can2_rx_msg);
		Can2_Hook(&can2_rx_msg);
	}
	
}

void Set_Shooter_Wave_Motors_Speed(float wave_wheel, float up_firction_speed, float down_firction_speed)
{
	Can_Send(2,
			CAN_SHOOTER_ALL_ID,
			Pid_Position_Calc(&motor_wave_speed_pid, wave_wheel, shooter_wave_motor.speed_rpm),
			0,
			Pid_Position_Calc(&friction_wheel_up_motor_speed_pid,
								up_firction_speed, friction_wheel_up_motor.speed_rpm),
			Pid_Position_Calc(&friction_wheel_down_motor_speed_pid, down_firction_speed, 
			 					friction_wheel_down_motor.speed_rpm));
}

//???????????????��2???????????????????????????????????
const Motor_measure_t *Get_Gimbal_Motor(void)
{
    return gimbal_motor;
}

const Motor_measure_t *Get_Shooter_Wave_Motor(void)
{
    return &shooter_wave_motor;
}
Motor_measure_t *Get_Firction_M3508_Up_Motor(void) {
    return &friction_wheel_up_motor;
}
Motor_measure_t *Get_Firction_M3508_Down_Motor(void) {
    return &friction_wheel_down_motor;
}

extern float easy_pid2_p, easy_pid2_i, easy_pid2_d;

//????????????????????PID????????????????????????????????????
void Set_Gimbal_Motors_Speed(float speed_yaw, float speed_pitch)
{
	// motor_pitch_speed_pid.kp = easy_pid2_p;
	// motor_pitch_speed_pid.ki = easy_pid2_i;
	// motor_pitch_speed_pid.kd = easy_pid2_d;
	Can_Send(2,
			 CAN_GIMBAL_ALL_ID,
			 Pid_Position_Calc(&motor_pitch_speed_pid, speed_pitch, gimbal_motor[1].speed_rpm),
			 0,
			 Pid_Position_Calc(&motor_yaw_speed_pid, speed_yaw, gimbal_motor[0].speed_rpm),
			 0);
}

//????????????YAW??????PID????????????????????0~360
float Calc_Yaw_Angle360_Pid(float tar_angle, float cur_angle)
{
	float yaw_tar_angle = tar_angle;
	float yaw_cur_angle = cur_angle;
	
	Handle_Angle360_PID_Over_Zero(&yaw_tar_angle, &yaw_cur_angle);
	return Pid_Position_Calc(&motor_yaw_angle_pid, yaw_tar_angle, yaw_cur_angle);
}
extern float easy_pid_p, easy_pid_i, easy_pid_d;

//????????????Pitch??????PID????????????????????0~8191
float Calc_Pitch_Angle8191_Pid(float tar_angle)
{
	// motor_pitch_angle_pid.kp = easy_pid_p;
	// motor_pitch_angle_pid.ki = easy_pid_i;
	// motor_pitch_angle_pid.kd = easy_pid_d;
	float pitch_tar_angle = tar_angle;
	float pitch_cur_angle = gimbal_motor[1].mechanical_angle;
	Pitch_Angle_Limit(&pitch_tar_angle,PITCH_DOWN_LIMIT,PITCH_UP_LIMIT);
	Handle_Angle8191_PID_Over_Zero(&pitch_tar_angle, &pitch_cur_angle);
	return Pid_Position_Calc(&motor_pitch_angle_pid, pitch_tar_angle, pitch_cur_angle);	
}


static void Can2_Hook(CanRxMsg *rx_message)
{
	switch (rx_message->StdId)
	{
		case CAN_3508_FRICTION_WHEEL_UP_ID:
			Calculate_Motor_Data(&friction_wheel_up_motor, rx_message);
			Shooter_Reload(0);
			break;
		case CAN_3508_FRICTION_WHEEL_DOWN_ID:
			Calculate_Motor_Data(&friction_wheel_down_motor, rx_message);
			Shooter_Reload(1);
			break;
		case CAN_3508_WAVE_ID:
			Calculate_Motor_Data(&shooter_wave_motor, rx_message);
			Shooter_Reload(2);
			break;
		
		case CAN_YAW_MOTOR_ID:
		{
			Calculate_Motor_Data(&gimbal_motor[0], rx_message);
			Gimbal_Reload(0);
			break;
		}
		case CAN_PITCH_MOTOR_ID:
		{
			Calculate_Motor_Data(&gimbal_motor[1], rx_message);
			Gimbal_Reload(1);
			break;
		}
		
		default:
			break;
	}
}


