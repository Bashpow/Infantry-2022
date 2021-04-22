#include "autoaim_task.h"

#include "autoaim.h"
#include "math2.h"
#include "detect_task.h"
#include "pid.h"

TaskHandle_t AutoaimTask_Handler;

static SemaphoreHandle_t autoaim_data_update_semaphore;  //串口1DMA接收信号量

void Autoaim_Task(void *pvParameters)
{
	autoaim_data_update_semaphore = xSemaphoreCreateBinary();

	vTaskDelay(200);

	DMA_ITConfig(DMA2_Stream1, DMA_IT_TC, ENABLE); //开启中断
	
	while(1)
	{
		if( xSemaphoreTake(autoaim_data_update_semaphore, 500) == pdTRUE )
		{
			//更新自瞄数据
			Auto_Aim_Calc();
			
			//更新陀螺仪状态
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
	// 释放二值信号量，发送接收到新数据标志，供前台程序查询
	xSemaphoreGiveFromISR(autoaim_data_update_semaphore, &pxHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}
