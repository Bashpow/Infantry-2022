#include "judge_task.h"

#include <string.h>
#include "usart8.h"
#include "judge_system.h"
#include "detect_task.h"

TaskHandle_t JudgeTask_Handler;

static const uint8_t *judge_buf;
static uint8_t judge_buf_copy[256];
static uint32_t judge_buf_len = 0;

void Judge_Task(void *pvParameters)
{
	Judge_System_Connect_List_Init();
	judge_buf = Get_Judge_Buf();
	
	vTaskDelay(200);
	
	USART_ITConfig(UART8, USART_IT_IDLE, ENABLE);
	
	while(1)
	{
		//等待任务通知
		xTaskNotifyWait(0x00, 0xFFFFFFFF, &judge_buf_len, portMAX_DELAY);
		
		//将接受的的原始数据拷贝为副本
		memcpy(judge_buf_copy, judge_buf, judge_buf_len);
		
		//更新裁判系统状态
		Detect_Reload(5);
		
		//下一行是打印所有原始数据
		// DEBUG_SHOWDATA1("judge_buf_len", judge_buf_len); //for(u8 i=0; i<judge_buf_len; i++){DEBUG_PRINT("%d ", judge_buf_copy[i]);} DEBUG_PRINT("\r\n");
		
		//解析裁判系统数据
		Analysis_Judge_System(judge_buf_copy, judge_buf_len);
		
	}
	
	//vTaskDelete(NULL);
	
}



void Notify_Judge_Task( uint32_t send_data )
{	
	/*向一个任务发送通知，xHandlingTask是该任务的句柄。*/
	xTaskNotifyFromISR(JudgeTask_Handler, send_data, eSetValueWithOverwrite, NULL);
}

