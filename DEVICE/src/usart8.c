#include "usart8.h"

#include "judge_task.h"
#include "judge_system.h"

#define JUDGE_SYSTEM_RX_MAX_LEN 256

static u8 uart8_rx_buf[JUDGE_SYSTEM_RX_MAX_LEN];
static u8 uart8_rx_length = 0;

void Usart8_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	DMA_InitTypeDef   DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8,ENABLE);

	GPIO_PinAFConfig(GPIOE,GPIO_PinSource0,GPIO_AF_UART8);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource1,GPIO_AF_UART8);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOE,&GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;				
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
	USART_Init(UART8, &USART_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = UART8_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	
	NVIC_Init(&NVIC_InitStructure);

	//串口8空闲中断（一帧数据接收完毕）
	USART_ITConfig(UART8, USART_IT_IDLE, DISABLE);

	USART_Cmd(UART8, ENABLE);
	USART_DMACmd(UART8, USART_DMAReq_Rx, ENABLE);
	USART_DMACmd(UART8, USART_DMAReq_Tx, ENABLE);

	/* DMA1流6 （串口8接收） */
	DMA_DeInit(DMA1_Stream6);
	while (DMA_GetCmdStatus(DMA1_Stream6) != DISABLE);
	DMA_InitStructure.DMA_Channel = DMA_Channel_5;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(UART8->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)uart8_rx_buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = JUDGE_SYSTEM_RX_MAX_LEN;
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
	DMA_Init(DMA1_Stream6, &DMA_InitStructure);

	/* DMA1流0 （串口8发送） */
	DMA_DeInit(DMA1_Stream0);
	while (DMA_GetCmdStatus(DMA1_Stream0) != DISABLE);
	DMA_InitStructure.DMA_Channel = DMA_Channel_5;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(UART8->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)NULL;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream0, &DMA_InitStructure);
}


void UART8_IRQHandler(void)
{
	if(USART_GetITStatus(UART8, USART_IT_IDLE) != RESET) 
	{
		//清除U8空闲中断
		//USART_ClearITPendingBit(UART8, USART_IT_IDLE);
		
		//关闭DMA
		DMA_Cmd(DMA1_Stream6, DISABLE);
		
		(void)UART8->SR;
		(void)UART8->DR;
		
		//计算接收到的数据长度
		uart8_rx_length = JUDGE_SYSTEM_RX_MAX_LEN - DMA_GetCurrDataCounter(DMA1_Stream6);
		
		//重设传输长度
		DMA_SetCurrDataCounter(DMA1_Stream6, JUDGE_SYSTEM_RX_MAX_LEN);
		
		//通知裁判系统任务解析
		Notify_Judge_Task(uart8_rx_length);
		
		//开启DMA
		DMA_Cmd(DMA1_Stream6,ENABLE);
		
	}
}

const uint8_t* Get_Judge_Buf(void)
{
	return uart8_rx_buf;
}

u8 Get_Judge_Buf_Len(void)
{
	return uart8_rx_length;
}

/**
 * @brief 串口8（裁判系统）使用dma发送数据
 * 
 * @param tx_buffer_address 数据首地址
 * @param tx_buffer_len 数据长度
 * @return uint8_t 发送成功---1，发送失败---0
 */
uint8_t Uart8_Transmit_Start(uint32_t tx_buffer_address, uint16_t tx_buffer_len)
{
	if ( (0 != DMA_GetCurrDataCounter(DMA1_Stream0)) && (DMA_SxCR_EN == (DMA1_Stream0->CR | DMA_SxCR_EN)))
	{
		return 0;
	}
	DMA_Cmd(DMA1_Stream0, DISABLE);
	DMA_ClearITPendingBit(DMA1_Stream0, DMA_IT_TCIF0);
	if (0 != tx_buffer_address)
	{
		DMA1_Stream0->M0AR = tx_buffer_address;
	}
    else
    {
        return 0;
    }
	DMA_SetCurrDataCounter(DMA1_Stream0, tx_buffer_len);
	DMA_Cmd(DMA1_Stream0, ENABLE);
	return 1;
}
