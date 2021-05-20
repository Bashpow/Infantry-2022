#include "init_task.h"
#include "infantry.h"
#include "led.h"
#include "delay.h"
#include "power_output.h"
#include "usart3.h"
#include "usart1.h"
#include "autoaim.h"
#include "usart7.h"
#include "usart8.h"
#include "can1.h"
#include "can2.h"
#include "timer2.h"
#include "timer6.h"
#include "buzzer.h"
#include "shell.h"

static void Print_Logo_2_Com(void);

void All_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	/* ��ʼ��LED */
	Led_Init();
	/* ��ʼ�������� */
	Buzzer_Init();
	/* ��ʼ������3 */
	Usart3_Init();
	Print_Logo_2_Com();
	INFO_PRINT("HLL 2021 New Infantry %s\r\n\r\n\r\n", VERSION);

	/* ��ʼ��ң����������1�� */
	Usart1_Init();
	
	/* ��ʼ��CAN1 */
	Can1_Init();

	/* ��ʼ��CAN2 */
	Can2_Init();
	
	/* ��ʼ����ʱ��2 */
	TIM2_PWM_Init(19999, 89);  //PWMƵ��Ϊ50Hz

	#if CHECK_CPU_USE
		TIM6_Init(50, 90);     //CPUʹ���ʲ鿴 20000Hz
	#endif
	
	/* ��ʼ���Զ���׼����6 */
	Auto_Aim_Init();
	
	/* ��ʼ�������Ǵ���7 */
	Uart7_Init();
	
	/* ��ʼ������ϵͳ����8 */
	Usart8_Init();
	
	/* ����24V������Ƴ�ʼ�� */
	Power_Ctrl_Init();
	POWER1_CTRL_ON;
	POWER2_CTRL_ON;
	POWER3_CTRL_ON;
	POWER4_CTRL_ON;
	
	/* Shell��ʼ�� */
	Shell_Init();

	INFO_LOG("All peripheral init.\r\n");
	Led_Flow_On();
}


static void Print_Logo_2_Com(void)
{
	INFO_PRINT("\r\n");
	INFO_PRINT("        *===*   *===*    *===*          *===*\r\n");
	INFO_PRINT("       /   /   /   /    /   /          /   /\r\n");
	INFO_PRINT("      /   *===*   /    /   /          /   /\r\n");
	INFO_PRINT("     /           /    /   /          /   /\r\n");
	INFO_PRINT("    /   *===*   /    /   *======*   /   *======*\r\n");
	INFO_PRINT("   /   /   /   /    /          /   /          /\r\n");
	INFO_PRINT("  *===*   *===*    *==========*   *==========*\r\n");
	INFO_PRINT("\r\n");
}


