#include "init_task.h"
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


static void Print_Logo_2_Com(void);

void All_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	/* 初始化LED */
	Led_Init();
	/* 初始化蜂鸣器 */
	Buzzer_Init();
	/* 初始化串口3 */
	Usart3_Init();
	Print_Logo_2_Com();
	INFO_PRINT("\r\nHLL_2021_New_Infantry_v0.9\r\n");
	/* 初始化遥控器（串口1） */
	Usart1_Init();
	/* 初始化CAN1 */
	Can1_Init();
	/* 初始化CAN2 */
	Can2_Init();
	
	/* 初始化定时器2 */
	TIM2_PWM_Init(19999, 89);  //PWM频率为50Hz

	#if CHECK_CPU_USE
		TIM6_Init(50, 90);     //CPU使用率查看 20000Hz
	#endif
	
	/* 初始化自动瞄准串口6 */
	Auto_Aim_Init();
	
	/* 初始化陀螺仪串口7 */
	Uart7_Init();
	
	/* 初始化裁判系统串口8 */
	Usart8_Init();
	
	/* 板载24V输出控制初始化 */
	Power_Ctrl_Init();
	POWER1_CTRL_ON;
	POWER2_CTRL_ON;
	POWER3_CTRL_ON;
	POWER4_CTRL_ON;
	
	INFO_PRINT("\r\nAll peripheral init.\r\n");
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


