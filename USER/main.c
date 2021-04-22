#include "stm32f4xx.h"

#include "FreeRTOS.h"
#include "task.h"

#include "init_task.h"
#include "start_task.h"


int main(void)
{
	//初始化所有外设
	All_Init();
	
	//创建开始任务
	xTaskCreate((TaskFunction_t )Start_Task,            //任务函数
							(const char*    )"start_task",          //任务名称
							(uint16_t       )128,                   //任务堆栈大小
							(void*          )NULL,                  //传递给任务函数的参数
							(UBaseType_t    )10,                    //任务优先级
							(TaskHandle_t*  )&StartTask_Handler);   //任务句柄                
	vTaskStartScheduler();                              //开启任务调度
	
	while(1);
}

