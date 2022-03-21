#include "power_output.h"
#include "stm32f4xx.h"

// 24V电源输出 PH2 PH3 PH4 PH5
// 红外模块5V专用电源输出 PG13

/**
 * @brief 可控电源（24V、5V）控制IO初始化
 * 
 */
void Power_Ctrl_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);

	GPIO_ResetBits(GPIOH, GPIO_Pin_2);
	GPIO_ResetBits(GPIOH, GPIO_Pin_3);
	GPIO_ResetBits(GPIOH, GPIO_Pin_4);
	GPIO_ResetBits(GPIOH, GPIO_Pin_5);
	GPIO_ResetBits(GPIOG, GPIO_Pin_13);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOH, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
}

