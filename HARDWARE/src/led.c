/*
 * 文件名：led.c
 * 描述 ：板载LED的操作
 * 定义了函数用于初始化LED
 * 定义了函数实现LED流水灯
 */

#include "led.h"
#include "delay.h"

/*
  函数名：Led_Init
  描述  ：初始化LED
  参数  ：无
  返回值：无
*/
void Led_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	
	/* 初始化LED_G */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	/* 初始化LED_R */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	LED_GREEN_OFF;
	LED_RED_OFF;
	
	/* 初始化板载LED阵列 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	LED_FLOW_OFF_PIN(GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8);
	
}

/*
  函数名：Led_All_Flash
  描述  ：LED炫酷的闪烁
  参数  ：无
  返回值：无
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
