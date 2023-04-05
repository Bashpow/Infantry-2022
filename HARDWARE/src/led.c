/*
 * �ļ�����led.c
 * ���� ������LED�Ĳ���
 * �����˺������ڳ�ʼ��LED
 * �����˺���ʵ��LED��ˮ��
 */

#include "led.h"
#include "delay.h"

/*
  ��������Led_Init
  ����  ����ʼ��LED
  ����  ����
  ����ֵ����
*/
void Led_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	
	/* ��ʼ��LED_G */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	/* ��ʼ��LED_R */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	LED_GREEN_OFF;
	LED_RED_OFF;
	
	/* ��ʼ������LED���� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	LED_FLOW_OFF_PIN(GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8);
	
}

/*
  ��������Led_All_Flash
  ����  ��LED�ſ����˸
  ����  ����
  ����ֵ����
*/
void Led_All_Flash(void)
{
	LED_GREEN_ON;
	delay_ms(100);
	LED_RED_ON;
	delay_ms(100);
	for(u8 i=0; i<8; i++)
	{
		LED_FLOW_ON(i);
		delay_ms(100);
	}
	
	for(u8 i=0; i<8; i++)
	{
		LED_FLOW_OFF(i);
		delay_ms(100);
	}
	LED_RED_OFF;
	delay_ms(100);
	LED_GREEN_OFF;
	delay_ms(100);
}

void Led_Flow_On(void)
{
	LED_GREEN_ON;
	delay_ms(30);
	LED_RED_ON;
	delay_ms(30);
	// for(int i=0; i<8; i++)
	// {
	// 	LED_FLOW_ON(i);
	// 	delay_ms(30);
	// }
	for(int8_t i=0; i<7; i+=2)
	{
		LED_FLOW_ON(i);
		delay_ms(30);
	}
	for(int8_t i=1; i<8; i+=2)
	{
		LED_FLOW_ON(i);
		delay_ms(30);
	}
}

void Led_Flow_Off(void)
{
	// for(u8 i=0; i<8; i++)
	// {
	// 	LED_FLOW_OFF(i);
	// 	delay_ms(30);
	// }
	for(u8 i=0; i<7; i+=2)
	{
		LED_FLOW_OFF(i);
		delay_ms(30);
	}
	for(u8 i=1; i<8; i+=2)
	{
		LED_FLOW_OFF(i);
		delay_ms(30);
	}
	LED_RED_OFF;
	delay_ms(30);
	LED_GREEN_OFF;
	delay_ms(30);
}
