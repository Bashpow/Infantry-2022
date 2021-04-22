#include "usart7.h"

//UART7 RX  PE7
//UART7 TX  PE8

#include "stdio.h"
#include "wt61c_task.h"

#include "usart3.h"

static uint8_t u7_rx_buf0[11];
static uint8_t u7_rx_buf1[11];

void Uart7_Init(void)
{
	/* 开启外设始终 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); //使能 GPIOE 时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, ENABLE); //使能 UART7 时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);  //使能 DMA1 时钟
	
	/* 设置引脚复用为串口 */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_UART7); //PE7 复用为 USART7
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_UART7); //PE8 复用为 USART7
	
	/* 初始化GPIO为复用功能 */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8; //GPIOD5 与 GPIOD6
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //速度 50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOE, &GPIO_InitStructure); //初始化 PD5，PD6
	
	/* 配置串口7 */
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;//波特率设置为 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为 8 位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收模式
	USART_Init(UART7, &USART_InitStructure); //初始化串口7
	
	/* 初始化串口7 DMA1数据流3通道5 DMA */
	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA1_Stream3); //复位初始化DMA数据流
	while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE) //确保DMA数据流复位完成
	{
	};
	/*usart2 rx对应dma1，通道4，数据流5*/
	DMA_InitStructure.DMA_Channel = DMA_Channel_5;
	/*设置DMA源：串口数据寄存器地址*/
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) & (UART7->DR);
	/*内存地址(要传输的变量的指针)*/
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)u7_rx_buf0;
	/*方向：从外设到内存*/
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	/*传输大小DMA_BufferSize=SENDBUFF_SIZE*/
	DMA_InitStructure.DMA_BufferSize = 11;
	/*外设地址不增*/
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	/*内存地址自增*/
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	/*外设数据单位*/
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	/*内存数据单位 8bit*/
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	/*DMA模式：循环模式*/
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	/*优先级：中*/
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	/*禁用FIFO*/
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	/*存储器突发传输 16个节拍*/
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	/*外设突发传输 1个节拍*/
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	/*配置DMA2的数据流5*/
	DMA_Init(DMA1_Stream3, &DMA_InitStructure);

	/* 开启DMA双缓冲模式 */
	DMA_DoubleBufferModeConfig(DMA1_Stream3, (uint32_t)&u7_rx_buf1, DMA_Memory_0); //内存0 即Buffer0先被传输
	DMA_DoubleBufferModeCmd(DMA1_Stream3, ENABLE);

	/*失能DMA*/
	DMA_Cmd(DMA1_Stream3, ENABLE);

	/* 开启DMA接收完成中断 */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, DISABLE);  //关闭DMA接收完成中断
	
	USART_DMACmd(UART7, USART_DMAReq_Rx, ENABLE);  //开启串口DMA模式
	USART_Cmd(UART7, ENABLE); //使能串口2
	
}

void DMA1_Stream3_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3))
	{
		DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3); //清除全部中断标志
		
		//通知陀螺仪任务
		Wt61c_Data_Update();
	}
}

u8 Get_Uart7_Available_Bufferx(void)
{	
	if(DMA_GetCurrentMemoryTarget(DMA1_Stream3))
	{
		return 0;
	}
	return 1;
}


const u8* Get_Uart7_Bufferx(u8 bufx)
{
	if(bufx == 0)
	{
		return u7_rx_buf0;
	}
	else if(bufx == 1)
	{
		return u7_rx_buf1;
	}
	return NULL;
}

void Uart7_DMA_Reset(void)
{	
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, DISABLE);
	USART_Cmd(UART7, DISABLE);
	DMA_Cmd(DMA1_Stream3, DISABLE);
	
	DMA_SetCurrDataCounter(DMA1_Stream3, 11);
	DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);
	
	DMA_Cmd(DMA1_Stream3, ENABLE);
	USART_Cmd(UART7, ENABLE);
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
}
