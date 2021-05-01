#include "shell_task.h"
#include "usart3.h"
#include "shell.h"

/* 相关变量的初始化 */
TaskHandle_t ShellTask_Handler;

static SemaphoreHandle_t shell_get_data_semaphore;  //串口7DMA接收信号量
static char * u3_rx_buf;

static void Shell_Str_Process(void);

void Shell_Task(void *pvParameters)
{
	//信号量初始化
	shell_get_data_semaphore = xSemaphoreCreateBinary();
	u3_rx_buf = Get_Usart3_Rx_Buffer();

	//等待200ms
	vTaskDelay(200);
	
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
	
	while(1)
	{
		if( xSemaphoreTake(shell_get_data_semaphore, portMAX_DELAY) == pdTRUE )
		{
			Shell_Str_Process(); //字符串预处理
			Shell_Command_Parse(u3_rx_buf);
		}
	}
	
	// vTaskDelete(NULL);
}

void Shell_Get_Data(void)
{
	BaseType_t pxHigherPriorityTaskWoken;
	// 释放二值信号量，发送接收到新数据标志，供前台程序查询
	xSemaphoreGiveFromISR(shell_get_data_semaphore, &pxHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

static void Shell_Str_Process(void)
{
	u8 u3_rx_len = Get_Usart3_Rx_Length();
	if(u3_rx_buf[u3_rx_len-2]=='\r' && u3_rx_buf[u3_rx_len-1]=='\n')
	{
		u3_rx_buf[u3_rx_len-2] = '\0';
	}
	else
	{
		u3_rx_buf[u3_rx_len] = '\0';
	}
}
