#include "usart6.h"

#include "autoaim_task.h"

static u8 usart6_rx_buf[10];

void Usart6_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef   dma;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
	
	/* 配置PG9、PG14为串口6 */
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource9,GPIO_AF_USART6);
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource14,GPIO_AF_USART6);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOG,&GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART6, &USART_InitStructure);

	USART_DMACmd(USART6,USART_DMAReq_Rx,ENABLE);
	USART_Cmd(USART6, ENABLE);
	
	/* 配置串口6DMA中断优先级 */
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* 配置DMA2流1通道5连接至串口6 */
	DMA_DeInit(DMA2_Stream1);
	while (DMA_GetCmdStatus(DMA2_Stream1) != DISABLE);
	dma.DMA_Channel= DMA_Channel_5;
	dma.DMA_PeripheralBaseAddr = (uint32_t)&(USART6->DR);
	dma.DMA_Memory0BaseAddr = (uint32_t)usart6_rx_buf;
	dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
	dma.DMA_BufferSize = 10;
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma.DMA_Mode = DMA_Mode_Circular;
	dma.DMA_Priority = DMA_Priority_High;
	dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma.DMA_MemoryBurst = DMA_Mode_Normal;
	dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream1, &dma);
	
	DMA_ITConfig(DMA2_Stream1, DMA_IT_TC, DISABLE); //不开启中断
	DMA_Cmd(DMA2_Stream1, ENABLE);
}

void DMA2_Stream1_IRQHandler(void)
{
	//printf("get_data\r\n");
	if (DMA_GetITStatus(DMA2_Stream1, DMA_IT_TCIF1))
	{
		DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);

		//通知自瞄任务（信号量）
		Autoaim_Data_Update();
	}
}

void Usart6_DMA_Reset(void)
{
	USART_Cmd(USART6, DISABLE); 
	DMA_Cmd(DMA2_Stream1, DISABLE);
	DMA_SetCurrDataCounter(DMA2_Stream1, 10);
	DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1);
	DMA_Cmd(DMA2_Stream1, ENABLE);
	USART_Cmd(USART6, ENABLE);
}

const uint8_t *Get_Usart6_Rx_Buf(void)
{
	return usart6_rx_buf;
}

