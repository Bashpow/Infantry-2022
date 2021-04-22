#include "stm32f4xx.h"

#include "FreeRTOS.h"
#include "task.h"

#include "init_task.h"
#include "start_task.h"


int main(void)
{
	//��ʼ����������
	All_Init();
	
	//������ʼ����
	xTaskCreate((TaskFunction_t )Start_Task,            //������
							(const char*    )"start_task",          //��������
							(uint16_t       )128,                   //�����ջ��С
							(void*          )NULL,                  //���ݸ��������Ĳ���
							(UBaseType_t    )10,                    //�������ȼ�
							(TaskHandle_t*  )&StartTask_Handler);   //������                
	vTaskStartScheduler();                              //�����������
	
	while(1);
}

