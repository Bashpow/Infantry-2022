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
#include "string.h" //memsetʹ��
#include "timer6.h"


//��ʼ������
TaskHandle_t StartTask_Handler;

#if CHECK_CPU_USE
static TaskHandle_t CPU_Task_Handle = NULL;
static void CPU_Task(void* parameter);
#endif

void Start_Task(void *pvParameters)
{
	taskENTER_CRITICAL();           //�����ٽ���
	
	//����ң������
	xTaskCreate((TaskFunction_t )Remoter_Task,             
							(const char*    )"remoter_task",
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )16,
							(TaskHandle_t*  )&RemoterTask_Handler);
	
	//����������ȡ����
	xTaskCreate((TaskFunction_t )Wt61c_Task,
							(const char*    )"wt61c_task",
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )15,
							(TaskHandle_t*  )&Wt61cTask_Handler);
	
	//��������������
	xTaskCreate((TaskFunction_t )Buzzer_Task,
							(const char*    )"buzzer_task",
							(uint16_t       )128,
							(void*          )NULL,
							(UBaseType_t    )3,
							(TaskHandle_t*  )&BuzzerTask_Handler);

	//����Can��������
	xTaskCreate((TaskFunction_t )Cansend_Task,
							(const char*    )"cansend_task",
							(uint16_t       )128,
							(void*          )NULL,
							(UBaseType_t    )25,
							(TaskHandle_t*  )&CansendTask_Handler);
						
	//������������
	xTaskCreate((TaskFunction_t )Chassis_Task,
							(const char*    )"chassis_task",
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )20,
							(TaskHandle_t*  )&ChassisTask_Handler);
						
	//������̨����(���ȼ����ڿ�������)
	xTaskCreate((TaskFunction_t )Gimbal_Task,
							(const char*    )"gimbal_task",
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )20,
							(TaskHandle_t*  )&GimbalTask_Handler);
							
	//���������������
	xTaskCreate((TaskFunction_t )Shooter_Task,
							(const char*    )"shooter_task",
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )19,
							(TaskHandle_t*  )&ShooterTask_Handler);
							
	//��������ϵͳ����
	xTaskCreate((TaskFunction_t )Judge_Task,
							(const char*    )"judge_task",   
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )9,
							(TaskHandle_t*  )&JudgeTask_Handler);
							
	//�����������ݻ�ȡ����
	xTaskCreate((TaskFunction_t )Autoaim_Task,
							(const char*    )"autoaim_task",   
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )8,
							(TaskHandle_t*  )&AutoaimTask_Handler);
	
	//������������
	xTaskCreate((TaskFunction_t )Detect_Task,     
							(const char*    )"detect_task",   
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )6,
							(TaskHandle_t*  )&DetectTask_Handler);

	//����Shell����
	xTaskCreate((TaskFunction_t )Shell_Task,     
							(const char*    )"shell_task",   
							(uint16_t       )256,
							(void*          )NULL,
							(UBaseType_t    )5,
							(TaskHandle_t*  )&ShellTask_Handler);
	
	//��������CPUʹ��������
	#if CHECK_CPU_USE
		xTaskCreate((TaskFunction_t )CPU_Task, /* ������ں��� */
								(const char*    )"CPU_Task",/* �������� */
								(uint16_t       )512,   /* ����ջ��С */
								(void*          )NULL,	/* ������ں������� */
								(UBaseType_t    )28,	    /* ��������ȼ� */
								(TaskHandle_t*  )&CPU_Task_Handle);/* ������ƿ�ָ�� */
	#endif

	Led_Flow_Off();
	INFO_LOG("All tasks creat.\r\n");

	taskEXIT_CRITICAL(); //�˳��ٽ���
	INFO_LOG("System run.\r\n\r\n");
	
	vTaskDelete(NULL);  //ɾ����ʼ����
}


#if CHECK_CPU_USE
static void CPU_Task(void* parameter)
{	
  uint8_t CPU_RunInfo[400];		//������������ʱ����Ϣ
  
  while (1)
  {
    memset(CPU_RunInfo,0,400);				//��Ϣ����������
    
    vTaskList((char *)&CPU_RunInfo);  //��ȡ��������ʱ����Ϣ
    
    printf("---------------------------------------------\r\n");
    printf("������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n");
    printf("%s", CPU_RunInfo);
    printf("---------------------------------------------\r\n");
    
    memset(CPU_RunInfo,0,400);				//��Ϣ����������
    
    vTaskGetRunTimeStats((char *)&CPU_RunInfo);
    
    printf("������       ���м���         ʹ����\r\n");
    printf("%s", CPU_RunInfo);
    printf("---------------------------------------------\r\n\n");
    vTaskDelay(500);   /* ��ʱ1000��tick */		
  }
}
#endif
