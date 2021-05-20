#include "shooter_task.h"

#include "timer2.h"
#include "can2_motor.h"
#include "remoter_task.h"
#include "buzzer_task.h"
#include "detect_task.h"
#include "judge_system.h"

#define COVER_OPEN()  S_PWM_OUT(2250);
#define COVER_CLOSE() S_PWM_OUT(1415);

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

//��������
TaskHandle_t ShooterTask_Handler;
static uint8_t shoot_key = 0;
static int16_t shooter_friction_speed = 1175;
const static Robot_mode_t* shooter_robot_mode;
const static Judge_data_t* judge_data;

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

	vTaskDelay(200);
	
	while(1)
	{
		
		/* Ħ�����ٶ�ģʽ,���տ��� */
		switch( FRIC_COVER_MODE )
		{
			case 0:
				friction_speed = FRICTION_STOP;
				COVER_CLOSE();
				break;
			case 1:
				friction_speed = FRICTION_SPEED_1;
				COVER_CLOSE();
				break;
			case 2:
				friction_speed = FRICTION_SPEED_2;
				COVER_CLOSE();
				break;
			case 3:
				friction_speed = FRICTION_STOP;
				COVER_OPEN();
				break;
			default:
				friction_speed = 0;
				COVER_CLOSE();
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

		/* ���ò����ٶ� */
		Set_Shooter_Wave_Motors_Speed(wave_speed);
		
		vTaskDelay(5);  //100HZ  ����1/10s 3����(10������)
	}
	
	//vTaskDelete(NULL);
	
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

//�ڲ���ϵͳ�յ��������ݺ���ã����Բ����жϲ���ϵͳ�Ƿ����ߣ�,���ڶ�̬���Ʒ����ӵ��ٶ�
void Shooter_Friction_Speed_Limit(void)
{
	if (judge_data->shoot_data.bullet_speed < ((float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit) - 2.0f) )
	{
		FRICTION_SPEED_1++;
	}

	if (judge_data->shoot_data.bullet_speed > ((float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit) - 1.2f))
	{
		FRICTION_SPEED_1 -= 1;
	}

	if (judge_data->shoot_data.bullet_speed > ((float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit) - 0.8f))
	{
		FRICTION_SPEED_1 -= 2;
	}
}

