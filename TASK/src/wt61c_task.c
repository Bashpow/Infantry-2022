#include "wt61c_task.h"

#include "usart7.h"
#include "detect_task.h"

static u8 Check_Wt61c_Data_Available(const u8* wt61c_raw_buf);

/* ��ر����ĳ�ʼ�� */
TaskHandle_t Wt61cTask_Handler;

static Wt61c_Data_t wt61c_data;
static const uint8_t* wt61c_rx_buf[2];
static uint32_t wt61c_available_bufx;

static SemaphoreHandle_t wt61c_data_update_semaphore;  //����7DMA�����ź���

void Wt61c_Task(void *pvParameters)
{
	//�ź�����ʼ��
	wt61c_data_update_semaphore = xSemaphoreCreateBinary();
	
	//��ȡjy901���������ǽ��ܵ�ԭʼ����
	wt61c_rx_buf[0] = Get_Uart7_Bufferx(0);
	wt61c_rx_buf[1] = Get_Uart7_Bufferx(1);
	
	//�ȴ�200ms
	vTaskDelay(200);
	
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);  //�ر�DMA��������ж�
	
	while(1)
	{
		if( xSemaphoreTake(wt61c_data_update_semaphore, portMAX_DELAY) == pdTRUE )
		{
		
			//��ȡ���ô���
			wt61c_available_bufx = 	Get_Uart7_Available_Bufferx();
			
	
			if( Check_Wt61c_Data_Available( wt61c_rx_buf[wt61c_available_bufx] ) )  //���Jy901�����Ƿ�Ϸ�
			{
			
				//����������״̬
				Detect_Reload(7);
			
				switch(wt61c_rx_buf[wt61c_available_bufx][1])  //�жϵ�ǰ���ܵ�����һ�֣����ٶȣ����ٶȣ��Ƕȣ����ݰ�
				{
				
					//��������Э�飬�����Ƕ����ݰ�
					case 0x53:					
						wt61c_data.angle.roll_x = ((short)(wt61c_rx_buf[wt61c_available_bufx][3]<<8|wt61c_rx_buf[wt61c_available_bufx][2]))/32768.0*180;
						wt61c_data.angle.pitch_y = ((short)(wt61c_rx_buf[wt61c_available_bufx][5]<<8|wt61c_rx_buf[wt61c_available_bufx][4]))/32768.0*180;
						wt61c_data.angle.yaw_z = ((short)(wt61c_rx_buf[wt61c_available_bufx][7]<<8|wt61c_rx_buf[wt61c_available_bufx][6]))/32768.0*180;
						if(wt61c_data.angle.yaw_z<0) wt61c_data.angle.yaw_z+=360;
						break;
				}
			
				//����ʱ�ڴ���������ᴫ�������ݣ����Խ������Խ����������ע�͵�
				//DEBUG_PRINT("angle  roll_x:%.2f\tpitch_y:%.2f\tyaw_z:%.2f\r\n", wt61c_data.angle.roll_x, wt61c_data.angle.pitch_y, wt61c_data.angle.yaw_z);

			
			}
		
			else //����У�鲻ͨ��
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
	// �ͷŶ�ֵ�ź��������ͽ��յ������ݱ�־����ǰ̨�����ѯ
	xSemaphoreGiveFromISR(wt61c_data_update_semaphore, &pxHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

/* ��ȡ�������������� */
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
