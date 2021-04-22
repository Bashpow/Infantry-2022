#include "autoaim_task.h"

#include "autoaim.h"
#include "math2.h"
#include "detect_task.h"
#include "pid.h"

TaskHandle_t AutoaimTask_Handler;

static SemaphoreHandle_t autoaim_data_update_semaphore;  //����1DMA�����ź���

void Autoaim_Task(void *pvParameters)
{
	autoaim_data_update_semaphore = xSemaphoreCreateBinary();

	vTaskDelay(200);

	DMA_ITConfig(DMA2_Stream1, DMA_IT_TC, ENABLE); //�����ж�
	
	while(1)
	{
		if( xSemaphoreTake(autoaim_data_update_semaphore, 500) == pdTRUE )
		{
			//������������
			Auto_Aim_Calc();
			
			//����������״̬
			Detect_Reload(4);
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
