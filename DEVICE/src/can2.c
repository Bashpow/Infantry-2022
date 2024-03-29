#include "can2.h"

void Can2_Init(void)
{
	CAN_InitTypeDef        can;
	CAN_FilterInitTypeDef  can_filter;
	GPIO_InitTypeDef       GPIO_InitStructure;
	NVIC_InitTypeDef       nvic;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);


	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12| GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 

	nvic.NVIC_IRQChannel = CAN2_RX0_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 5;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	CAN_DeInit(CAN2);
	CAN_StructInit(&can);

	can.CAN_TTCM = DISABLE;
	can.CAN_ABOM = DISABLE;    
	can.CAN_AWUM = DISABLE;    
	can.CAN_NART = DISABLE;    
	can.CAN_RFLM = DISABLE;    
	can.CAN_TXFP = ENABLE; 


	can.CAN_Mode = CAN_Mode_Normal;
	can.CAN_SJW  = CAN_SJW_1tq;
	can.CAN_BS1 = 8;
	can.CAN_BS2 = 4;
	can.CAN_Prescaler = 3;   //CAN BaudRate 42/(1+9+4)/3=1Mbps
	CAN_Init(CAN2, &can);

	can_filter.CAN_FilterNumber=14;
	can_filter.CAN_FilterMode=CAN_FilterMode_IdMask;
	can_filter.CAN_FilterScale=CAN_FilterScale_32bit;
	can_filter.CAN_FilterIdHigh=0x0000;
	can_filter.CAN_FilterIdLow=0x0000;
	can_filter.CAN_FilterMaskIdHigh=0x0000;
	can_filter.CAN_FilterMaskIdLow=0x0000;
	can_filter.CAN_FilterFIFOAssignment=0;
	can_filter.CAN_FilterActivation=ENABLE;
	CAN_FilterInit(&can_filter);

	CAN_ITConfig(CAN2,CAN_IT_FMP0,ENABLE);
}

void Can2_Send_4Msg(uint32_t id, int16_t data1, int16_t data2, int16_t data3, int16_t data4)
{
	CanTxMsg TxMessage;
	TxMessage.StdId = id;
	TxMessage.IDE = CAN_Id_Standard;
	TxMessage.RTR = CAN_RTR_Data;
	TxMessage.DLC = 0x08;
	TxMessage.Data[0] = data1 >> 8;
	TxMessage.Data[1] = data1;
	TxMessage.Data[2] = data2 >> 8;
	TxMessage.Data[3] = data2;
	TxMessage.Data[4] = data3 >> 8;
	TxMessage.Data[5] = data3;
	TxMessage.Data[6] = data4 >> 8;
	TxMessage.Data[7] = data4;

	CAN_Transmit(CAN2, &TxMessage);
}

