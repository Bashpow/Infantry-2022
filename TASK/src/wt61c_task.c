#include "wt61c_task.h"

#include "usart7.h"
#include "detect_task.h"

static u8 Check_Wt61c_Data_Available(const u8* wt61c_raw_buf);

/* 相关变量的初始化 */
TaskHandle_t Wt61cTask_Handler;

static Wt61c_Data_t wt61c_data;
static const uint8_t* wt61c_rx_buf[2];
static uint32_t wt61c_available_bufx;

static SemaphoreHandle_t wt61c_data_update_semaphore;  //串口7DMA接收信号量

void Wt61c_Task(void *pvParameters)
{
	//信号量初始化
	wt61c_data_update_semaphore = xSemaphoreCreateBinary();
	
	//获取jy901六轴陀螺仪接受的原始数据
	wt61c_rx_buf[0] = Get_Uart7_Bufferx(0);
	wt61c_rx_buf[1] = Get_Uart7_Bufferx(1);
	
	//等待200ms
	vTaskDelay(200);
	
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);  //关闭DMA接收完成中断
	
	while(1)
	{
		if( xSemaphoreTake(wt61c_data_update_semaphore, portMAX_DELAY) == pdTRUE )
		{
		
			//获取可用串口
			wt61c_available_bufx = 	Get_Uart7_Available_Bufferx();
			
	
			if( Check_Wt61c_Data_Available( wt61c_rx_buf[wt61c_available_bufx] ) )  //检查Jy901数据是否合法
			{
			
				//更新陀螺仪状态
				Detect_Reload(7);
			
				switch(wt61c_rx_buf[wt61c_available_bufx][1])  //判断当前接受的是哪一种（加速度，角速度，角度）数据包
				{
				
					//依照数据协议，解析角度数据包
					case 0x53:					
						wt61c_data.angle.roll_x = ((short)(wt61c_rx_buf[wt61c_available_bufx][3]<<8|wt61c_rx_buf[wt61c_available_bufx][2]))/32768.0*180;
						wt61c_data.angle.pitch_y = ((short)(wt61c_rx_buf[wt61c_available_bufx][5]<<8|wt61c_rx_buf[wt61c_available_bufx][4]))/32768.0*180;
						wt61c_data.angle.yaw_z = ((short)(wt61c_rx_buf[wt61c_available_bufx][7]<<8|wt61c_rx_buf[wt61c_available_bufx][6]))/32768.0*180;
						if(wt61c_data.angle.yaw_z<0) wt61c_data.angle.yaw_z+=360;
						break;
				}
			
				//调试时在串口输出六轴传感器数据，调试结束可以讲下两条语句注释掉
				//DEBUG_PRINT("angle  roll_x:%.2f\tpitch_y:%.2f\tyaw_z:%.2f\r\n", wt61c_data.angle.roll_x, wt61c_data.angle.pitch_y, wt61c_data.angle.yaw_z);

			
			}
		
			else //数据校验不通过
			{
				Uart7_DMA_Reset();
				INFO_LOG("WT61C error.\r\n");
				vTaskDelay(2);
			}
			
		}
		
	}
	
}

void Wt61c_Data_Update(void)
{
	BaseType_t pxHigherPriorityTaskWoken;
	// 释放二值信号量，发送接收到新数据标志，供前台程序查询
	xSemaphoreGiveFromISR(wt61c_data_update_semaphore, &pxHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

/* 获取六轴陀螺仪数据 */
const Wt61c_Data_t* Get_Wt61c_Data(void)
{
	return &wt61c_data;
}

static u8 Check_Wt61c_Data_Available(const u8* wt61c_raw_buf)
{
	u8 sum_add = 0;
	if(wt61c_raw_buf[0] != 0x55)
	{
		return 0;
	}
	for(u8 i=0; i<10; i++)
	{
		sum_add += wt61c_raw_buf[i];
	}
	if(wt61c_raw_buf[10] != sum_add)
	{
		return 0;
	}
	return 1;
}
