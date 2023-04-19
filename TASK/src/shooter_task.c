#include "shooter_task.h"

#include "math2.h"
#include "timer2.h"
#include "pid.h"
#include "can2_motor.h"
#include "power_output.h"
#include "remoter_task.h"
#include "buzzer_task.h"
#include "detect_task.h"
#include "judge_system.h"

#define COVER_OPEN()  S_PWM_OUT(2250);
#define COVER_CLOSE() S_PWM_OUT(999);

#define FRICTION_MIN     1160
#define FRICTION_MAX     1280
#define FRICTION_STOP    800
#define FRICTION_SPEED_1 shooter_friction_speed
#define FRICTION_SPEED_2 1280
extern float easy_pid_p, easy_pid_i, easy_pid_d;

#define FRICTION3508_MIN     4300
#define FRICTION3508_MAX     8250
#define FRICTION3508_STOP    0
#define FRICTION3508_SPEED_1 (shooter_friction3508_speed)
// #define FRICTION3508_SPEED_2 (easy_pid_p) //调试时使用
#define FRICTION3508_SPEED_2 ((FRICTION3508_MAX+FRICTION3508_MIN)/2)

#define WAVE_SPEED_ONE_SHOOT 5000
#define WAVE_SPEED_THREE_SHOOT 6800
#define WAVE_SPEED_1 2200
#define WAVE_SPEED_2 5000

#define ONE_SHOOT_CNT 7
#define THREE_SHOOT_CNT 20

#define FRIC_COVER_MODE   shooter_robot_mode->fric_cover_mode
#define SHOOT_KEY shoot_key

#define CALC_FRICTION_SPEED_PID() Pid_Position_Calc(&motor_friction_speed_pid, ((float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit) - 2.0f), judge_data->shoot_data.bullet_speed)
#define CALC_FRICTION3508_SPEED_PID() \
	Pid_Position_Calc(&motor_friction3508_speed_pid, \
	((float)(judge_data->ext_game_robot_status_t.shooter_id1_17mm_speed_limit) - 0.35f), \
	judge_data->ext_shoot_data_t.bullet_speed)

//锟斤拷锟斤拷锟斤拷锟斤拷
TaskHandle_t ShooterTask_Handler;
static uint8_t shoot_key = 0;
static int16_t shooter_friction_speed = 1170;
static int16_t shooter_friction3508_speed = 4600;
const static Robot_mode_t* shooter_robot_mode;
const static Judge_data_t* judge_data;
Pid_Position_t motor_friction_speed_pid = NEW_POSITION_PID(2.0f, 0.1, 0, 8, 12, 0, 1000, 500);
static Pid_Position_t motor_friction3508_speed_pid = NEW_POSITION_PID(5.0f, 0.1, 0, 8, 12, 0, 1000, 500);
//static int16_t motor_friction_speed_max;
static int16_t motor_friction3508_speed_max;
Motor_measure_t *firction_m3508_up_wheel;
Motor_measure_t *firction_m3508_down_wheel;

static void Shoot_End_Friction3508_Speed_Subtract(uint16_t minus_speed);

void Shooter_Task(void *pvParameters)
{
	int16_t wave_speed = 0;  //锟斤拷锟斤拷锟劫讹拷
	int16_t friction_speed = 0;  //摩锟斤拷锟斤拷锟劫讹拷
	
	uint8_t shoot_count = 0;  //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	uint8_t shooting_sign = 0;  //锟斤拷锟斤拷锟街?

	shooter_robot_mode = Get_Robot_Mode_Point();
	judge_data = Get_Judge_Data();
	firction_m3508_up_wheel = Get_Firction_M3508_Up_Motor();
	firction_m3508_down_wheel = Get_Firction_M3508_Down_Motor();
	const Rc_ctrl_t *rc_pt = Get_Remote_Control_Point();
	CanRxMsg *can2_rx_msg = GetCan2_RXD_Msg();
	vTaskDelay(200);

	for(;;)
	{
		switch( FRIC_COVER_MODE )
		{
			case 0:
				friction_speed = FRICTION3508_STOP;
				COVER_CLOSE();
				LASER_OFF;
				break;
			case 1:
				friction_speed = FRICTION3508_SPEED_1;
				COVER_CLOSE();
				LASER_ON;
				break;
			case 2:
				friction_speed = FRICTION3508_SPEED_2;
				COVER_CLOSE();
				LASER_ON;
				break;
			case 3:
				friction_speed = FRICTION3508_STOP;
				COVER_OPEN();
				LASER_OFF;
				break;
			default:
				friction_speed = 0;
				COVER_CLOSE();
				LASER_OFF;
				break;
		}
		
		if( FRIC_COVER_MODE==1 ||  FRIC_COVER_MODE==2)
		{
			if(shoot_count == 0)
			{
				switch( shoot_key )
				{
					case 1:
						wave_speed = -WAVE_SPEED_ONE_SHOOT;
						shoot_count = ONE_SHOOT_CNT;
						shoot_key = 0;
						break;
					
					case 2:
						wave_speed = -WAVE_SPEED_THREE_SHOOT;
						shoot_count = THREE_SHOOT_CNT;
						shoot_key = 0;
						break;
					
					case 3:
						wave_speed = -WAVE_SPEED_1;
						shooting_sign = 1;
						shoot_count = 0;
						break;
					
					case 4:
						wave_speed = -WAVE_SPEED_2;
						shoot_count = 0;
						break;
					
					default:
						wave_speed=0;
						shoot_count=0;
				}
			}
			else
			{
				if(shooting_sign)
				{
					Shoot_End_Friction3508_Speed_Subtract(5);
					shooting_sign = 0;
				}
				shoot_count--;
			}
			
		}
		else
		{
			wave_speed=0;
		}
		
		// 老版本snail电机用
		// T_PWM_OUT(friction_speed);
		// U_PWM_OUT(friction_speed);

		Set_Shooter_Wave_Motors_Speed(wave_speed , (float)friction_speed, (float)-friction_speed);
		vTaskDelay(5);
	}
	
	//vTaskDelete(NULL);
	
}

// 设置发射按键
void Set_Shoot_key(u8 key)
{
	shoot_key = key;
}

// 每一轮发射结束后，降低摩擦轮目标速度
static void Shoot_End_Friction3508_Speed_Subtract(uint16_t minus_speed)
{
	if(Get_Module_Online_State(5))
	{
		if (judge_data->ext_shoot_data_t.bullet_speed >
			((float)(judge_data->ext_game_robot_status_t.shooter_id1_17mm_speed_limit) - 1.0f))
		{
			FRICTION_SPEED_1 -= minus_speed;
		}
	}
}

//若裁判系统上线，且弹速限制改变时，重设基础速度
void ShooterFriction3508SpeedBaseLimit(uint16_t speed_limit)
{
	static uint16_t last_speed_limit = 0;
	if(last_speed_limit != speed_limit)
	{
		last_speed_limit = speed_limit;
		FRICTION3508_SPEED_1 = 
			ShooterSpeedToFrictionSpeed( judge_data->ext_game_robot_status_t.shooter_id1_17mm_speed_limit - 1 );
		motor_friction3508_speed_max = 
			ShooterSpeedToFrictionSpeed( judge_data->ext_game_robot_status_t.shooter_id1_17mm_speed_limit + 1 );
		Int16_Constrain(&FRICTION3508_SPEED_1, FRICTION3508_MIN, FRICTION3508_MAX);
		Int16_Constrain(&motor_friction3508_speed_max, FRICTION3508_MIN, FRICTION3508_MAX);
	}
}

// 裁判系统收到发射数据后的回调，用于动态限制摩擦轮速度
void JudgeSystemGetShootMsgCallback(unsigned short id) {
	if(judge_data->ext_shoot_data_t.bullet_speed >
		(float)(judge_data->ext_game_robot_status_t.shooter_id1_17mm_speed_limit))
	{
		motor_friction3508_speed_max = FRICTION3508_SPEED_1;
	}
	float test = CALC_FRICTION3508_SPEED_PID();
	FRICTION3508_SPEED_1 += Int16_Limit( test, -50, 10 );
	Int16_Constrain(&FRICTION3508_SPEED_1, 
		ShooterSpeedToFrictionSpeed(judge_data->ext_game_robot_status_t.shooter_id1_17mm_speed_limit-1), 
		ShooterSpeedToFrictionSpeed(judge_data->ext_game_robot_status_t.shooter_id1_17mm_speed_limit+1));
	Int16_Constrain(&FRICTION3508_SPEED_1, FRICTION3508_MIN, motor_friction3508_speed_max);
}

// 摩擦轮速度对应大概的电机速度
int32_t ShooterSpeedToFrictionSpeed(int16_t shooter_speed) {
	switch (shooter_speed)
	{
	case 13: return 4450;
	case 14: return 4600;
	case 15: return 4750;
	case 16: return 5000;
	case 17: return 5150;
	case 18: return 5380;
	case 19: return 5530;
	case 20: return 5800;
	case 21: return 5950;
	case 22: return 6060;
	case 23: return 6190;
	case 24: return 6450;
	case 25: return 6610;
	case 26: return 6795;
	case 27: return 7100;
	case 28: return 7300;
	case 29: return 7550;
	case 30: return 7780;
	case 31: return 7960;
	case 32: return 8250;
	default:
		return 4450;
	}
	// return 0;
}

// !!!
// !!! 以下不用管
//以下是老版本程序，snail电机用

/*

// speed -> pwm
// y = -0.0019x4 + 0.1998x3 - 6.8204x2 + 99.437x + 648.01
static uint16_t Shooter_Friction_Speed_To_Pwm(float speed)
{
	float pwm = (-0.0019f*speed*speed*speed*speed) + (0.1998f*speed*speed*speed) + (-6.8204f*speed*speed) + (99.437f*speed) + 648.01f;
	return (uint16_t)pwm;
}

void Shoot_End_Friction_Speed_Subtract(uint16_t minus_speed)
{
	if(Get_Module_Online_State(5))
	{
		if (judge_data->shoot_data.bullet_speed > ((float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit) - 2.0f))
		{
			FRICTION_SPEED_1 -= minus_speed;
		}
	}
}

#define SHOOTER_FRICTION_SPEED_TO_PWM(speed)  Shooter_Friction_Speed_To_Pwm(speed)
void Shooter_Friction_Speed_Base_Limit(uint16_t speed_limit)
{
	static uint16_t last_speed_limit = 0;
	if(last_speed_limit != speed_limit)
	{
		last_speed_limit = speed_limit;
		FRICTION_SPEED_1 = SHOOTER_FRICTION_SPEED_TO_PWM( ((float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit) - 5.0f) );
		motor_friction_speed_max = SHOOTER_FRICTION_SPEED_TO_PWM( (float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit)-1.0f );
		Int16_Constrain(&FRICTION_SPEED_1, FRICTION_MIN, FRICTION_MAX);
		Int16_Constrain(&motor_friction_speed_max, FRICTION_MIN, FRICTION_MAX);
	}
}

void Shooter_Friction_Speed_Limit(void)
{
	if(judge_data->shoot_data.bullet_speed > (float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit))
	{
		motor_friction_speed_max = FRICTION_SPEED_1;
	}
	FRICTION_SPEED_1 += Int16_Limit( CALC_FRICTION_SPEED_PID(), -12, 2 );
	Int16_Constrain(&FRICTION_SPEED_1, FRICTION_MIN, motor_friction_speed_max);  //摩锟斤拷锟斤拷锟劫讹拷锟斤拷锟斤拷锟斤拷锟斤拷
}

*/
