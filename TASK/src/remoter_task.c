#include "remoter_task.h"

#include "remoter.h"
#include "usart1.h"
#include "usart3.h"
#include "judge_system.h"
#include "buzzer_task.h"
#include "shooter_task.h"
#include "detect_task.h"

#define S1_VALUE       remote_controller.rc.s1
#define S2_VALUE       remote_controller.rc.s2
#define OLD_S1_VALUE   last_time_rc.rc.s1
#define OLD_S2_VALUE   last_time_rc.rc.s2
#define KEY_VALUE      remote_controller.key.value
#define OLD_KEY_VALUE  last_time_rc.key.value

#define S1_AT(a)       (S1_VALUE == (a))
#define S2_AT(a)       (S2_VALUE == (a))

#define S1_CHANGED_TO(a,b) ((OLD_S1_VALUE == (a)) && (S1_VALUE == (b)))
#define S2_CHANGED_TO(a,b) ((OLD_S2_VALUE == (a)) && (S2_VALUE == (b)))

#define KEY_CLICKED(key)   (RC_KEY_PRESSED(KEY_VALUE,key) && (!RC_KEY_PRESSED(OLD_KEY_VALUE,key)))

/* �������� */
static void Robot_Rc_Mode_Change_Control(void);
static void Shoot_Key_Control(void);
void Switch_Mouse_Key_Change(Rc_ctrl_t* rc, Rc_ctrl_t* last_rc, Robot_mode_t* robot_mode);

/* ���� */
TaskHandle_t RemoterTask_Handler;
static Rc_ctrl_t remote_controller;  //���λ�ȡ��ң��������
static Rc_ctrl_t last_time_rc;  //��һ�ε�ң��������
static Robot_mode_t robot_mode;  //������ģʽ
static const uint8_t* rc_rx_buf[2];  //����1����ԭʼ��������ָ��
static SemaphoreHandle_t rc_data_update_semaphore;  //����1DMA�����ź���
static const Judge_data_t* judge_data;

void Remoter_Task(void *pvParameters)
{
	static uint8_t rx_available_bufx;

	//��ֵ�ź�����ʼ��
	rc_data_update_semaphore = xSemaphoreCreateBinary();
	
	//ȡ��ң����DBUSԭʼ����ָ��
	rc_rx_buf[0] = Get_Rc_Bufferx(0);
	rc_rx_buf[1] = Get_Rc_Bufferx(1);
	
	judge_data = Get_Judge_Data();

	//����ң��������
	Rc_Data_Reset(&remote_controller);
	Rc_Data_Reset(&last_time_rc);
	
	//��ʼ��������ģʽ
	{
		robot_mode.control_device=1;  //�ٿ��豸ѡ�� 1 ����  2ң����
		robot_mode.mouse_key_chassis_mode=1; //1���� 2С���� 3����
		robot_mode.mouse_key_gimbal_mode=1; //1�ֶ� 2���� 3����
		robot_mode.fric_cover_mode=0; //Ħ����ģʽ 0�ر� 1���� 2���� 3���ո�
		robot_mode.shoot_mode=1; //���ģʽ 1���� 2������ 3�����ٶ�һ 4�����ٶȶ�
		robot_mode.motion_mode=5; //1���̸��� 2С���� 3������� 4����С���� 5����
		robot_mode.shoot_key=0; //������� 1���� 2������ 3�����ٶ�һ 4�����ٶȶ�
	}
	
	vTaskDelay(200);
	
	//��������1DMA��������ж�
	DMA_ITConfig(DMA2_Stream2, DMA_IT_TC, ENABLE);

	while(1)
	{
		
		//�ȴ��ź�������ʱʱ��50ms
		if( xSemaphoreTake(rc_data_update_semaphore, 500) == pdTRUE )
		{
			/* ��ȡ��ǰң��������ԭʼ���� */
			rx_available_bufx = Get_Rc_Available_Bufferx();

			/* ����ң�������� */
			Parse_Remoter_Data(rc_rx_buf[rx_available_bufx], &remote_controller);

			/* ���ң�������ݣ��Ƿ�Ϸ�������Ϸ���ȡ���� */
			if(Remoter_Data_Check(&remote_controller))
			{
				//���ֵ�ǰң�������ݲ���
				Rc_Data_Copy(&remote_controller, &last_time_rc);
				//���ô���1��DMA
				Usart1_DMA_Reset();
				
				DEBUG_ERROR(100);
			}
			
			/* ����ң����״̬ */
			Detect_Reload(0);
			
			/* ������ģʽ�任��Ӧ */
			Robot_Rc_Mode_Change_Control();
			
			/* ��Ӧ���̿��� */
			Switch_Mouse_Key_Change(&remote_controller, &last_time_rc, &robot_mode);

			/* �����ť */
			Shoot_Key_Control();
			
			/* ���汾��ң����״̬ */
			Rc_Data_Copy(&last_time_rc, &remote_controller);
			
		}
		
		//���ȴ��ź�����ʱ
		else
		{
			Rc_Data_Reset(&remote_controller);
			Rc_Data_Reset(&last_time_rc);
		}
		
	}
	
	//vTaskDelete(NULL);
}

/*
  ��������Rc_Data_Update
	����  ����ң��������DMA���պ���ʱ���ã�����֪ͨ������remoter task��ң�������ݽ������
  ����  ����
  ����ֵ����
*/
void Rc_Data_Update(void)
{
	BaseType_t pxHigherPriorityTaskWoken;
	// �ͷŶ�ֵ�ź��������ͽ��յ������ݱ�־����ǰ̨�����ѯ
	xSemaphoreGiveFromISR(rc_data_update_semaphore, &pxHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

/*
  ��������Clear_Shoot_Key
	����  ���ڱ�Ҫʱ��������ťֵ
  ����  ����
  ����ֵ����
*/
void Clear_Shoot_Key(void)
{
	robot_mode.shoot_key = 0;
}

void Fric_Reset(void)
{
	robot_mode.fric_cover_mode = 0;
}

/*
  ��������Get_Remote_Control_Point
	����  ����ȡң��������
  ����  ����
  ����ֵ��Rc_ctrl_t�ṹ�����
*/
const Rc_ctrl_t *Get_Remote_Control_Point(void)
{
    return &remote_controller;
}

/*
  ��������Get_Robot_Mode_Point
	����  ����ȡ������ģʽ����
  ����  ����
  ����ֵ��Robot_mode_t�ṹ�����
*/
const Robot_mode_t *Get_Robot_Mode_Point(void)
{
    return &robot_mode;
}

/* ��Ӧң���������л�ģʽ */
static void Robot_Rc_Mode_Change_Control(void)
{
	/* �ٿ��豸ѡ�� */
	if(S2_CHANGED_TO(3,2))
	{
		robot_mode.control_device++;
		if(robot_mode.control_device==3) robot_mode.control_device=1;
		Set_Beep_Time(robot_mode.control_device, 1000, 55);
	}
	
	/* ������̨ģʽ */
	if(S2_CHANGED_TO(3,1))
	{
		robot_mode.motion_mode++;
		if(robot_mode.motion_mode==6) robot_mode.motion_mode=1;
		Set_Beep_Time(robot_mode.motion_mode, 1200, 50);
	}

	if(robot_mode.control_device != 2)
	{
		return;
	}

	/* Ħ�����ٶ�ģʽ���ոǿ��� */
	if(S1_CHANGED_TO(3,2))
	{
		robot_mode.fric_cover_mode++;
		if(robot_mode.fric_cover_mode==4) robot_mode.fric_cover_mode=0;
		Set_Beep_Time(robot_mode.fric_cover_mode+1, 800, 60);
	}

	/* ���ģʽѡ���� */
	if(S1_CHANGED_TO(3,1))
	{
		robot_mode.shoot_mode++;
		if(robot_mode.shoot_mode==5) robot_mode.shoot_mode=1;
		Set_Beep_Time(robot_mode.shoot_mode, 600, 65);
	}
}

/* ��Ӧ�����л�ģʽ */
void Switch_Mouse_Key_Change(Rc_ctrl_t* rc, Rc_ctrl_t* last_rc, Robot_mode_t* robot_mode)
{
	//�ж��ǲ��Ǽ���ģʽ
	if(robot_mode->control_device != 1)
	{
		return;
	}

	//����ģʽ(��סshift����С����)
	if( RC_KEY_PRESSED(KEY_VALUE,KEY_SHIFT) )
	{
		robot_mode->mouse_key_chassis_mode = 2;
	}
	else
	{
		if(robot_mode->mouse_key_chassis_mode != 3)
			robot_mode->mouse_key_chassis_mode = 1;
	}

	//��̨ģʽ
	if(KEY_CLICKED(KEY_CTRL))
	{
		if(robot_mode->mouse_key_gimbal_mode == 1)
		{
			robot_mode->mouse_key_gimbal_mode = 2;
			Set_Beep_Time(4, 1200, 50);
		}
		else
		{
			robot_mode->mouse_key_gimbal_mode = 1;
			Set_Beep_Time(3, 1200, 50);
		}
	}

	//����ģʽ
	if( KEY_CLICKED(KEY_Z) )
	{
		robot_mode->mouse_key_chassis_mode = 3;
		robot_mode->mouse_key_gimbal_mode = 3;
		Set_Beep_Time(5, 1200, 50);
	}

	//����ģʽ���
	if(robot_mode->mouse_key_chassis_mode == 3 && robot_mode->mouse_key_gimbal_mode != 3)
	{
		robot_mode->mouse_key_chassis_mode = 1;
	}
	if(robot_mode->mouse_key_chassis_mode != 3 && robot_mode->mouse_key_gimbal_mode == 3)
	{
		robot_mode->mouse_key_gimbal_mode = 1;
	}

	//����ģʽ
	if(KEY_CLICKED(KEY_Q))
	{
		if(robot_mode->fric_cover_mode == 3)
		{
			robot_mode->fric_cover_mode = 0;
			Set_Beep_Time(1, 800, 60);
		}
		else
		{
			robot_mode->fric_cover_mode = 3;
			Set_Beep_Time(4, 800, 60);
		}
	}

	//Ħ����ģʽ
	if(KEY_CLICKED(KEY_E))
	{
		if(robot_mode->fric_cover_mode == 1)
		{
			robot_mode->fric_cover_mode = 0;
			Set_Beep_Time(1, 800, 60);
		}
		else
		{
			robot_mode->fric_cover_mode = 1;
			Set_Beep_Time(2, 800, 60);
		}
	}
}

/* ��Ӧ���ģʽ */
static void Shoot_Key_Control(void)
{
	static u8 mouse_first_press = 1;

	//ң����ģʽ
	if(robot_mode.control_device == 2)
	{
		/* �����ť */
		if (robot_mode.shoot_mode == 1 || robot_mode.shoot_mode == 2)
		{
			if (last_time_rc.rc.ch4 < 120 && remote_controller.rc.ch4 > 120)
			{
				robot_mode.shoot_key = robot_mode.shoot_mode;
			}
		}
		else
		{
			if (remote_controller.rc.ch4 > 120)
			{
				robot_mode.shoot_key = robot_mode.shoot_mode;
			}
			else
			{
				robot_mode.shoot_key = 0;
			}
		}
	}
	
	//����ģʽ
	else
	{
		if( Is_Id1_17mm_Excess_Heat(judge_data) == 0 )  //�ж��Ƿ�����
		{
			if(remote_controller.mouse.press_l)
			{
				if(mouse_first_press)
				{
					First_Shoot_Friction_Speed_Subtract(2);
					mouse_first_press = 0;
				}
				robot_mode.shoot_mode = 3;
				robot_mode.shoot_key = 3;
			}
			else
			{
				mouse_first_press = 1;
				robot_mode.shoot_key = 0;
			}

			if( remote_controller.mouse.press_r && (!last_time_rc.mouse.press_r) )
			{
				First_Shoot_Friction_Speed_Subtract(2);
				robot_mode.shoot_mode = 2;
				robot_mode.shoot_key = 2;
			}
		}
		else
		{
			robot_mode.shoot_key = 0;
		}

	}

}

