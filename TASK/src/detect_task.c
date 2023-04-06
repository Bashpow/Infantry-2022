#include "detect_task.h"
#include "listen.h"
#include "led.h"
#include "usart3.h"

#define MODULE_REONLINE(index) (module_status[index].time_out_flag==0 && module_status[index].old_time_out_flag==1)
#define MODULE_OFFLINE(index)  (module_status[index].time_out_flag==1 && module_status[index].old_time_out_flag==0)

static void Detect_Task_Init(void);

// �ֱ����8��ģ��
static Module_status_t module_status[8];

TaskHandle_t DetectTask_Handler;

void Detect_Task(void *pvParameters)
{
	Detect_Task_Init();
	vTaskDelay(800);
	
	for(;;)
	{
		for(u8 i=0; i<8; i++)
		{
			//����ģ��״̬ˢ��
			Module_Status_Listen(&module_status[i]);
			
			if(MODULE_REONLINE(i))
			{
				LED_FLOW_ON(i);
				INFO_LOG("Module%d Online.\r\n\r\n", i);
			}
			else if(MODULE_OFFLINE(i))
			{
				LED_FLOW_OFF(i);
				INFO_LOG("Module%d Offline.\r\n\r\n", i);
			}
		}
		
		LED_GREEN_TOGGLE;
		vTaskDelay(250);
	}
	//vTaskDelete(NULL);
}

/**
 * @brief ����ģ���ʼ��
 * 
 */
static void Detect_Task_Init(void)
{
	for(uint8_t i=0; i<8; i++)
	{
		Module_Status_Init(&module_status[i], 5, NULL, NULL);
	}
	module_status[5].reload_cnt = 10;
}

/**
 * @brief ��ȡģ������״̬
 * 
 * @param id ģ��id
 * @return uint8_t ģ������---1u��ģ������---0u
 */
uint8_t Get_Module_Online_State(uint8_t id)
{
	return Get_Module_State(&module_status[id]);
}

/**
 * @brief ˢ��ģ��
 * 
 * @param id ģ��id
 */
void Detect_Reload(uint8_t id)
{
	Module_Status_Reload(&module_status[id]);
}


/**
 * @brief ˢ�µ��̵��״̬
 * 
 * @param motor_index must be 0~3,�ֱ��Ӧ�����ĸ����
 */
void Classis_Reload(const int8_t motor_index)
{
	// bit0-3��Ӧ4�����
	static uint8_t chassis_motor_state = 0u;

	// ˢ�¶�Ӧbit
	if (motor_index >= 0 && motor_index <= 3)
	{
		chassis_motor_state |= (1u << motor_index);
	}
	else
	{
		return;
	}
	// �ж������Ƿ����ߣ���������ˢ��״̬
	if ((chassis_motor_state & 0x0F) == 0x0F)
	{
		chassis_motor_state = 0;
		Module_Status_Reload(&module_status[CHASSIS_MOTOR]);
	}
}

/**
 * @brief ˢ����̨���״̬
 * 
 * @param motor_index must be 0~1,�ֱ��Ӧ�����������
 */
void Gimbal_Reload(int8_t motor_index)
{
	static uint8_t gimbal_motor[2] = {0, 0};
	if(motor_index>=0 && motor_index<=1)
	{
		gimbal_motor[motor_index] = 1;
	}
	else
	{
		return;
	}
	if(gimbal_motor[0] && gimbal_motor[1])
	{
		gimbal_motor[0] = 0;
		gimbal_motor[1] = 0;
		Module_Status_Reload(&module_status[GIMBAL_MOTOR]);
	}
}

/**
 * @brief ˢ�·������3���״̬
 * 
 * @param motor_index must be 0~2,�ֱ��Ӧ3�����
 */
void Shooter_Reload(const int8_t motor_index)
{
	static uint8_t shooter_motor_state = 0u; // bit0-3��Ӧ4�����
	if (motor_index >= 0 && motor_index <= 2) {
		shooter_motor_state |= (1u << motor_index); // ˢ�¶�Ӧbit
	} else {
		return;
	}	
	if ((shooter_motor_state & 0x07) == 0x07) { // �ж������Ƿ����ߣ���������ˢ��״̬
		shooter_motor_state = 0;
		Module_Status_Reload(&module_status[SHOOTER_MOTOR]);
	}
}
