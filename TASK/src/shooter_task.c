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
#define COVER_CLOSE() S_PWM_OUT(1415);

#define FRICTION_MIN     1160
#define FRICTION_MAX     1280
#define FRICTION_STOP    800
#define FRICTION_SPEED_1 shooter_friction_speed
#define FRICTION_SPEED_2 1280

#define WAVE_SPEED_ONE_SHOOT 5000
#define WAVE_SPEED_THREE_SHOOT 6800
#define WAVE_SPEED_1 2200
#define WAVE_SPEED_2 5000

#define ONE_SHOOT_CNT 7
#define THREE_SHOOT_CNT 20

#define FRIC_COVER_MODE   shooter_robot_mode->fric_cover_mode
#define SHOOT_KEY shoot_key

#define CALC_FRICTION_SPEED_PID() Pid_Position_Calc(&motor_friction_speed_pid, ((float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit) - 2.0f), judge_data->shoot_data.bullet_speed)

//��������
TaskHandle_t ShooterTask_Handler;
static uint8_t shoot_key = 0;
static int16_t shooter_friction_speed = 1170;
const static Robot_mode_t* shooter_robot_mode;
const static Judge_data_t* judge_data;
static Pid_Position_t motor_friction_speed_pid = NEW_POSITION_PID(2.0f, 0.1, 0, 8, 12, 0, 1000, 500); //�����ٶ�PID
static int16_t motor_friction_speed_max;
Motor_measure_t *firction_m3508_up_wheel;
Motor_measure_t *firction_m3508_down_wheel;
//��������
static void Shoot_End_Friction_Speed_Subtract(uint16_t minus_speed);

void Shooter_Task(void *pvParameters)
{
	int16_t wave_speed = 0;  //�����ٶ�
	int16_t friction_speed = 0;  //Ħ�����ٶ�
	
	uint8_t shoot_count = 0;  //��������������
	uint8_t shooting_sign = 0;  //�����־

	shooter_robot_mode = Get_Robot_Mode_Point();
	judge_data = Get_Judge_Data();
	firction_m3508_up_wheel = Get_Firction_M3508_Up_Motor();
	firction_m3508_down_wheel = Get_Firction_M3508_Down_Motor();
	const Rc_ctrl_t *rc_pt = Get_Remote_Control_Point();
	vTaskDelay(200);
	CanRxMsg *can2_rx_msg = GetCan2_RXD_Msg();

	for(;;)
	{
		
		/* Ħ�����ٶ�ģʽ,���տ��� */
		switch( FRIC_COVER_MODE )
		{
			case 0:
				friction_speed = FRICTION_STOP;
				COVER_CLOSE();
				LASER_OFF;
				break;
			case 1:
				friction_speed = FRICTION_SPEED_1;
				COVER_CLOSE();
				LASER_ON;
				break;
			case 2:
				friction_speed = FRICTION_SPEED_2;
				COVER_CLOSE();
				LASER_ON;
				break;
			case 3:
				friction_speed = FRICTION_STOP;
				COVER_OPEN();
				LASER_OFF;
				break;
			default:
				friction_speed = 0;
				COVER_CLOSE();
				LASER_OFF;
				break;
		}
		
		/* ���������ť */
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
					Shoot_End_Friction_Speed_Subtract(2);
					shooting_sign = 0;
				}
				shoot_count--;
			}
			
		}
		else
		{
			wave_speed=0;
		}
		
		/* ����Ħ���� */
		T_PWM_OUT(friction_speed);
		U_PWM_OUT(friction_speed);

		float up_speed = rc_pt->rc.ch0;
		float down_speed = rc_pt->rc.ch1;
		// printf("ch1: %d, ch2: %d\r\n",rc_pt->rc.ch0, rc_pt->rc.ch1);
		// printf("%d %d %d %d\r\n", firction_m3508_up_wheel->mechanical_angle, firction_m3508_up_wheel->speed_rpm,
		// firction_m3508_down_wheel->mechanical_angle,firction_m3508_down_wheel->speed_rpm);
		/* ���ò����ٶ� */
		Set_Shooter_Wave_Motors_Speed(wave_speed ,up_speed, down_speed);
		// printf("id: %d; \r\n",can2_rx_msg->StdId);
		vTaskDelay(5);  //100HZ  ����1/10s 3����(10������)
	}
	
	//vTaskDelete(NULL);
	
}

// speed -> pwm
// y = -0.0019x4 + 0.1998x3 - 6.8204x2 + 99.437x + 648.01
static uint16_t Shooter_Friction_Speed_To_Pwm(float speed)
{
	float pwm = (-0.0019f*speed*speed*speed*speed) + (0.1998f*speed*speed*speed) + (-6.8204f*speed*speed) + (99.437f*speed) + 648.01f;
	return (uint16_t)pwm;
}

//����������ɿ���������Ħ�����ٶȽ���
static void Shoot_End_Friction_Speed_Subtract(uint16_t minus_speed)
{
	if(Get_Module_Online_State(5))
	{
		if (judge_data->shoot_data.bullet_speed > ((float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit) - 2.0f))
		{
			FRICTION_SPEED_1 -= minus_speed;
		}
	}
}

//���������ť
void Set_Shoot_key(u8 key)
{
	shoot_key = key;
}

#define SHOOTER_FRICTION_SPEED_TO_PWM(speed)  Shooter_Friction_Speed_To_Pwm(speed)
//�ڲ���ϵͳ�յ��ӵ��ٶ�����ʱ���ã����Բ����жϲ���ϵͳ�Ƿ����ߣ������ӵ��ٶ����Ʊ��ʱ����Ħ�����ٶ�
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

//�ڲ���ϵͳ�յ��������ݺ���ã����Բ����жϲ���ϵͳ�Ƿ����ߣ�,���ڶ�̬���Ʒ����ӵ��ٶ�
void Shooter_Friction_Speed_Limit(void)
{
	if(judge_data->shoot_data.bullet_speed > (float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit))
	{
		motor_friction_speed_max = FRICTION_SPEED_1;
	}
	FRICTION_SPEED_1 += Int16_Limit( CALC_FRICTION_SPEED_PID(), -12, 2 );
	Int16_Constrain(&FRICTION_SPEED_1, FRICTION_MIN, motor_friction_speed_max);  //Ħ�����ٶ���������
}

