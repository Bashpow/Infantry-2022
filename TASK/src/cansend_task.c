#include "cansend_task.h"
#include "can1.h"
#include "can2.h"

/* 变量定义 */
TaskHandle_t CansendTask_Handler;
static QueueHandle_t can_send_queue;

typedef struct
{
	uint8_t canx;
	uint32_t id;
	int16_t data1;
	int16_t data2;
	int16_t data3;
	int16_t data4;
}Can_send_t;

void Cansend_Task(void *pvParameters)
{
	//消息队列初始化
	can_send_queue = xQueueCreate( 3, sizeof( Can_send_t ) );
	
	//蜂鸣器播放数据
	Can_send_t can_send_msg;
	
	vTaskDelay(120);
	
	while(1)
	{
		
		if( xQueueReceive(can_send_queue, &can_send_msg, portMAX_DELAY) == pdPASS)
		{
			if(can_send_msg.canx == 1)
			{
				Can1_Send_4Msg(can_send_msg.id, can_send_msg.data1, can_send_msg.data2, can_send_msg.data3, can_send_msg.data4);
			}
			else if(can_send_msg.canx == 2)
			{
				Can2_Send_4Msg(can_send_msg.id, can_send_msg.data1, can_send_msg.data2, can_send_msg.data3, can_send_msg.data4);
			}
		}
		else
		{
			DEBUG_LOG("Can send task error!!!\r\n");
		}
		
	}
	
	
}

//设置蜂鸣器
BaseType_t Can_Send(uint8_t canx, uint32_t id, int16_t data1, int16_t data2, int16_t data3, int16_t data4)
{
	Can_send_t can_send_msg;
	can_send_msg.canx = canx;
	can_send_msg.id = id;
	can_send_msg.data1 = data1;
	can_send_msg.data2 = data2;
	can_send_msg.data3 = data3;
	can_send_msg.data4 = data4;
	return xQueueSend(can_send_queue, &can_send_msg, 0);
}
