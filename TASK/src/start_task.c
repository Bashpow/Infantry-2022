#include "start_task.h"

#include "led.h"
#include "buzzer_task.h"
#include "remoter_task.h"
#include "wt61c_task.h"
#include "cansend_task.h"
#include "chassis_task.h"
#include "gimbal_task.h"
#include "shooter_task.h"
#include "autoaim_task.h"
#include "judge_task.h"
#include "detect_task.h"
#include "shell_task.h"
#include "string.h" //memset使用
#include "timer6.h"


//开始任务句柄
TaskHandle_t StartTask_Handler;

#if CHECK_CPU_USE
static TaskHandle_t CPU_Task_Handle = NULL;
static void CPU_Task(void* parameter);
#endif

void Start_Task(void *pvParameters)
{
	taskENTER_CRITICAL();           //进入临界区
	
	//创建遥控任务
	xTaskCreate((TaskFunction_t )Remoter_Task,             
							(const char*    )"remoter_task",
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )16,
							(TaskHandle_t*  )&RemoterTask_Handler);
	
	//创建陀螺仪取任务
	xTaskCreate((TaskFunction_t )Wt61c_Task,
							(const char*    )"wt61c_task",
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )15,
							(TaskHandle_t*  )&Wt61cTask_Handler);
	
	//创建蜂鸣器任务
	xTaskCreate((TaskFunction_t )Buzzer_Task,
							(const char*    )"buzzer_task",
							(uint16_t       )128,
							(void*          )NULL,
							(UBaseType_t    )3,
							(TaskHandle_t*  )&BuzzerTask_Handler);

	//创建Can发送任务
	xTaskCreate((TaskFunction_t )Cansend_Task,
							(const char*    )"cansend_task",
							(uint16_t       )128,
							(void*          )NULL,
							(UBaseType_t    )25,
							(TaskHandle_t*  )&CansendTask_Handler);
						
	//创建底盘任务
	xTaskCreate((TaskFunction_t )Chassis_Task,
							(const char*    )"chassis_task",
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )20,
							(TaskHandle_t*  )&ChassisTask_Handler);
						
	//创建云台任务(优先级等于空闲任务)
	xTaskCreate((TaskFunction_t )Gimbal_Task,
							(const char*    )"gimbal_task",
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )20,
							(TaskHandle_t*  )&GimbalTask_Handler);
							
	//创建发射机构任务
	xTaskCreate((TaskFunction_t )Shooter_Task,
							(const char*    )"shooter_task",
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )19,
							(TaskHandle_t*  )&ShooterTask_Handler);
							
	//创建裁判系统任务
	xTaskCreate((TaskFunction_t )Judge_Task,
							(const char*    )"judge_task",   
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )9,
							(TaskHandle_t*  )&JudgeTask_Handler);
							
	//创建自瞄数据获取任务
	xTaskCreate((TaskFunction_t )Autoaim_Task,
							(const char*    )"autoaim_task",   
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )8,
							(TaskHandle_t*  )&AutoaimTask_Handler);
	
	//创建监听任务
	xTaskCreate((TaskFunction_t )Detect_Task,     
							(const char*    )"detect_task",   
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )6,
							(TaskHandle_t*  )&DetectTask_Handler);

	//创建Shell任务
	xTaskCreate((TaskFunction_t )Shell_Task,     
							(const char*    )"shell_task",   
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )5,
							(TaskHandle_t*  )&ShellTask_Handler);
	
	//创建监听CPU使用率任务
	#if CHECK_CPU_USE
		xTaskCreate((TaskFunction_t )CPU_Task, /* 任务入口函数 */
								(const char*    )"CPU_Task",/* 任务名字 */
								(uint16_t       )512,   /* 任务栈大小 */
								(void*          )NULL,	/* 任务入口函数参数 */
								(UBaseType_t    )28,	    /* 任务的优先级 */
								(TaskHandle_t*  )&CPU_Task_Handle);/* 任务控制块指针 */
	#endif

	Led_Flow_Off();
	INFO_LOG("All tasks creat.\r\n");

	taskEXIT_CRITICAL(); //退出临界区
	INFO_LOG("System run.\r\n\r\n");
	
	vTaskDelete(NULL);  //删除开始任务
}


#if CHECK_CPU_USE
static void CPU_Task(void* parameter)
{	
  uint8_t CPU_RunInfo[400];		//保存任务运行时间信息
  
  while (1)
  {
    memset(CPU_RunInfo,0,400);				//信息缓冲区清零
    
    vTaskList((char *)&CPU_RunInfo);  //获取任务运行时间信息
    
    printf("---------------------------------------------\r\n");
    printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
    printf("%s", CPU_RunInfo);
    printf("---------------------------------------------\r\n");
    
    memset(CPU_RunInfo,0,400);				//信息缓冲区清零
    
    vTaskGetRunTimeStats((char *)&CPU_RunInfo);
    
    printf("任务名       运行计数         使用率\r\n");
    printf("%s", CPU_RunInfo);
    printf("---------------------------------------------\r\n\n");
    vTaskDelay(500);   /* 延时1000个tick */		
  }
}
#endif
