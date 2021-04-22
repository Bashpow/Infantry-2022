#ifndef __LED_H
#define __LED_H

#include "sys.h"

#define LED_RED PEout(11)
#define LED_GREEN PFout(14)
#define LED_FLOW(led_num) PGout(led_num)

#define LED_RED_ON     GPIOE->BSRRH = GPIO_Pin_11
#define LED_RED_OFF    GPIOE->BSRRL = GPIO_Pin_11
#define LED_RED_TOGGLE GPIOE->ODR ^= GPIO_Pin_11

#define LED_GREEN_ON     GPIOF->BSRRH = GPIO_Pin_14
#define LED_GREEN_OFF    GPIOF->BSRRL = GPIO_Pin_14
#define LED_GREEN_TOGGLE GPIOF->ODR ^= GPIO_Pin_14

#define LED_FLOW_ON(led_num) GPIOG->BSRRH = GPIO_Pin_8 >> led_num
#define LED_FLOW_OFF(led_num) GPIOG->BSRRL = GPIO_Pin_8 >> led_num
#define LED_FLOW_TOGGLE(led_num) GPIOG->ODR ^= GPIO_Pin_8 >> led_num

#define LED_FLOW_ON_PIN(GPIO_Pin) GPIOG->BSRRH = GPIO_Pin
#define LED_FLOW_OFF_PIN(GPIO_Pin) GPIOG->BSRRL = GPIO_Pin
#define LED_FLOW_TOGGLE_PIN(GPIO_Pin) GPIOG->ODR ^= GPIO_Pin

void Led_Init(void);
void Led_All_Flash(void);
void Led_Flow_On(void);
void Led_Flow_Off(void);

#endif
