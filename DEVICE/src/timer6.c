#include "timer6.h"

void TIM6_Init(uint16_t arr, uint16_t psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM6, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM6, DISABLE);

	TIM_DeInit(TIM6);
	TIM_TimeBaseInitStructure.TIM_Period = arr - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc - 1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStructure);
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM6, ENABLE);
}

/* 用于统计运行时间 */
volatile uint32_t CPU_RunTime = 0UL;

void TIM6_DAC_IRQHandler(void)
{
    if( TIM_GetITStatus( TIM6, TIM_IT_Update )!= RESET )
    {
		TIM_ClearITPendingBit(TIM6 , TIM_FLAG_Update);
		CPU_RunTime++;
    }
}
