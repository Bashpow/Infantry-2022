/*
 * 文件名：usart1.c
 * 描述 ：初始化串口1
 * 串口1与遥控器接收器相连接
 * 串口1使用DMA接收数据
 * 使用Usart1_DMA_Reset函数可以重新设置串口1DMA传输
 * 定义了局部变量数组rc_rx_buff[18]用于储存遥控器接收的原始数据
*/

#include "usart1.h"

#include "stdio.h"

#define RC_RX_BUF_NUM 18

static uint8_t rc_rx_buf0[RC_RX_BUF_NUM];
static uint8_t rc_rx_buf1[RC_RX_BUF_NUM];
/*
  函数名：Usart1_Init
  描述  ：初始化串口1
  参数  ：无
  返回值：无
*/
void Usart1_Init(void)
{
	/* 开启相关时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
 
	/* IO口、串口配置 */
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_USART1);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 100000;
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_Even;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx;
	USART_Init(USART1, &USART_InitStructure);
	
	USART_DMACmd(USART1,USART_DMAReq_Rx, ENABLE);
	USART_Cmd(USART1, ENABLE);
	
	/* 串口1DMA配置 */
	DMA_InitTypeDef DMA_InitStructure;
	DMA_Cmd(DMA2_Stream2, DISABLE);
	while (DMA2_Stream2->CR & DMA_SxCR_EN);
	DMA_DeInit(DMA2_Stream2);
	DMA_InitStructure.DMA_Channel= DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)rc_rx_buf0;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = RC_RX_BUF_NUM;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream2, &DMA_InitStructure);

	/* 开启DMA双缓冲模式 */
	DMA_DoubleBufferModeConfig(DMA2_Stream2, (uint32_t)&rc_rx_buf1, DMA_Memory_0); //内存0 即Buffer0先被传输
	DMA_DoubleBufferModeCmd(DMA2_Stream2, ENABLE);

	/* DMA2流5中断优先级配置 */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_Cmd(DMA2_Stream2, ENABLE);
	
}

/*
  函数名：Get_Rc_Buf
  描述  ：获取rc_rx_buf
  参数  ：bufx
  返回值：串口1接收数组指针
*/
const u8* Get_Rc_Bufferx(u8 bufx)
{
	if(bufx == 0)
	{
		return rc_rx_buf0;
	}
	else if(bufx == 1)
	{
		return rc_rx_buf1;
	}
	return NULL;
}

u8 Get_Rc_Available_Bufferx(void)
{
	if(DMA_GetCurrentMemoryTarget(DMA2_Stream2))
	{
		return 0;
	}
	return 1;
}

/*
  函数名：Usart1_DMA_Reset
  描述  ：重启串口1和串口1DMA
  参数  ：无
  返回值：无
*/
void Usart1_DMA_Reset(void)
{
	DMA_ITConfig(DMA2_Stream2, DMA_IT_TC, DISABLE);
	DMA_Cmd(DMA2_Stream2, DISABLE);
	USART_Cmd(USART1, DISABLE);
	DMA_SetCurrDataCounter(DMA2_Stream2, RC_RX_BUF_NUM);	
	DMA_Cmd(DMA2_Stream2, ENABLE);
	USART_Cmd(USART1, ENABLE);
	DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TCIF2);
	DMA_ITConfig(DMA2_Stream2, DMA_IT_TC, ENABLE);
}


