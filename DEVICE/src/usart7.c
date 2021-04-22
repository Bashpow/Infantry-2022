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
	/* ��������ʼ�� */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); //ʹ�� GPIOE ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, ENABLE); //ʹ�� UART7 ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);  //ʹ�� DMA1 ʱ��
	
	/* �������Ÿ���Ϊ���� */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_UART7); //PE7 ����Ϊ USART7
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_UART7); //PE8 ����Ϊ USART7
	
	/* ��ʼ��GPIOΪ���ù��� */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8; //GPIOD5 �� GPIOD6
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //�ٶ� 50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOE, &GPIO_InitStructure); //��ʼ�� PD5��PD6
	
	/* ���ô���7 */
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;//����������Ϊ 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ 8 λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//��ģʽ
	USART_Init(UART7, &USART_InitStructure); //��ʼ������7
	
	/* ��ʼ������7 DMA1������3ͨ��5 DMA */
	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA1_Stream3); //��λ��ʼ��DMA������
	while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE) //ȷ��DMA��������λ���
	{
	};
	/*usart2 rx��Ӧdma1��ͨ��4��������5*/
	DMA_InitStructure.DMA_Channel = DMA_Channel_5;
	/*����DMAԴ���������ݼĴ�����ַ*/
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) & (UART7->DR);
	/*�ڴ��ַ(Ҫ����ı�����ָ��)*/
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)u7_rx_buf0;
	/*���򣺴����赽�ڴ�*/
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	/*�����СDMA_BufferSize=SENDBUFF_SIZE*/
	DMA_InitStructure.DMA_BufferSize = 11;
	/*�����ַ����*/
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	/*�ڴ��ַ����*/
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	/*�������ݵ�λ*/
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	/*�ڴ����ݵ�λ 8bit*/
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	/*DMAģʽ��ѭ��ģʽ*/
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	/*���ȼ�����*/
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	/*����FIFO*/
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	/*�洢��ͻ������ 16������*/
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	/*����ͻ������ 1������*/
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	/*����DMA2��������5*/
	DMA_Init(DMA1_Stream3, &DMA_InitStructure);

	/* ����DMA˫����ģʽ */
	DMA_DoubleBufferModeConfig(DMA1_Stream3, (uint32_t)&u7_rx_buf1, DMA_Memory_0); //�ڴ�0 ��Buffer0�ȱ�����
	DMA_DoubleBufferModeCmd(DMA1_Stream3, ENABLE);

	/*ʧ��DMA*/
	DMA_Cmd(DMA1_Stream3, ENABLE);

	/* ����DMA��������ж� */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, DISABLE);  //�ر�DMA��������ж�
	
	USART_DMACmd(UART7, USART_DMAReq_Rx, ENABLE);  //��������DMAģʽ
	USART_Cmd(UART7, ENABLE); //ʹ�ܴ���2
	
}

void DMA1_Stream3_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3))
	{
		DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3); //���ȫ���жϱ�־
		
		//֪ͨ����������
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
