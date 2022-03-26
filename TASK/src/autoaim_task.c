#include "autoaim_task.h"

#include "autoaim.h"
#include "math2.h"
#include "detect_task.h"
#include "pid.h"
#include "usart3.h"
#include "usart6.h"

static SemaphoreHandle_t autoaim_data_update_semaphore;  //����1DMA�����ź���

TaskHandle_t AutoaimTask_Handler;

void Autoaim_Task(void *pvParameters)
{
	autoaim_data_update_semaphore = xSemaphoreCreateBinary();
	vTaskDelay(200);
	DMA_ITConfig(DMA2_Stream1, DMA_IT_TC, ENABLE); //�����ж�
	for(;;)
	{
		if( xSemaphoreTake(autoaim_data_update_semaphore, 500) == pdTRUE )
		{
			if(Auto_Aim_Calc())
			{
				//����������״̬
				Detect_Reload(AUTO_AIM);
			}
			else
			{
				DEBUG_ERROR(601);
				Usart6_DMA_Reset();
			}
		}
		else
		{
			Reset_Auto_Aim_Msg();
		}
	}
	
}

void Autoaim_Data_Update(void)
{
	BaseType_t pxHigherPriorityTaskWoken;
	// �ͷŶ�ֵ�ź��������ͽ��յ������ݱ�־����ǰ̨�����ѯ
	xSemaphoreGiveFromISR(autoaim_data_update_semaphore, &pxHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}
