#include "shooter_task.h"

#include "timer2.h"
#include "can2_motor.h"
#include "remoter_task.h"
#include "buzzer_task.h"
#include "judge_system.h"

#define FRIC_COVER_MODE   shooter_robot_mode->fric_cover_mode
#define SHOOT_KEY         shooter_robot_mode->shoot_key

#define COVER_OPEN()  S_PWM_OUT(2250);
#define COVER_CLOSE() S_PWM_OUT(1415);

#define FRICTION_STOP    800
//#define FRICTION_SPEED_1 1185
#define FRICTION_SPEED_1 shoot_speed
#define FRICTION_SPEED_2 1280


//��������
TaskHandle_t ShooterTask_Handler;

static int16_t shoot_speed = 1175;
const static Robot_mode_t* shooter_robot_mode;
const static Judge_data_t* judge_data;

void Shooter_Task(void *pvParameters)
{
	int16_t wave_speed = 0;  //�����ٶ�
	int16_t friction_speed = 0;  //Ħ�����ٶ�
	
	int16_t one_three_shoot_speed = 0; //�����������ٶ�
	uint16_t shoot_count=0;  //��������������
	
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
				switch( SHOOT_KEY )
				{
					case 1:
						wave_speed = -5000;
						one_three_shoot_speed = -5000;
						shoot_count=3;
						Clear_Shoot_Key();
						break;
					
					case 2:
						wave_speed = -6800;
						one_three_shoot_speed = -6800;
						shoot_count=8;
						Clear_Shoot_Key();
						break;
					
					case 3:
						wave_speed=-2200;
						shoot_count=0;
						break;
					
					case 4:
						wave_speed=-5000;
						shoot_count=0;
						break;
					
					default:
						wave_speed=0;
						shoot_count=0;
				}
			}
			else
			{
				wave_speed = one_three_shoot_speed;
				shoot_count--;
			}
			
		}
		else
		{
			wave_speed=0;
		}
		
		/* ���ò����ٶ� */
		Set_Shooter_Wave_Motors_Speed(wave_speed);
		
		/* ����Ħ���� */
		U_PWM_OUT(friction_speed);
		V_PWM_OUT(friction_speed);
		
		vTaskDelay(10);  //100HZ  ����1/10s 3����(10������)
	}
	
	
	//vTaskDelete(NULL);
	
}

//�ڲ���ϵͳ�յ��������ݺ����,���ڶ�̬���Ʒ����ӵ��ٶ�
void Shooter_Friction_Speed_Limit(void)
{
	// if (judge_data->shoot_data.bullet_speed < ((float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit) -10.0f) )
	// {
	// 	shoot_speed += 10;
	// }
	if (judge_data->shoot_data.bullet_speed < ((float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit) - 2.0f) )
	{
		shoot_speed++;
	}

	if (judge_data->shoot_data.bullet_speed > ((float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit) - 1.2f))
	{
		shoot_speed -= 1;
	}

	if (judge_data->shoot_data.bullet_speed > ((float)(judge_data->game_robot_status.shooter_id1_17mm_speed_limit) - 1.6f))
	{
		shoot_speed -= 2;
	}
}

//�״�������������������Ҽ���Ħ�����ٶȽ���
void First_Shoot_Friction_Speed_Subtract(uint16_t minus_speed)
{
	if (judge_data->shoot_data.bullet_speed > 11.2f)
	{
		shoot_speed -= minus_speed;
	}
}
