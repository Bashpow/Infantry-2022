#ifndef __POWER_OUTPUT_H
#define __POWER_OUTPUT_H

#define LASER_ON GPIO_SetBits(GPIOG, GPIO_Pin_13)
#define POWER1_CTRL_ON GPIO_SetBits(GPIOH, GPIO_Pin_2)
#define POWER2_CTRL_ON GPIO_SetBits(GPIOH, GPIO_Pin_3)
#define POWER3_CTRL_ON GPIO_SetBits(GPIOH, GPIO_Pin_4)
#define POWER4_CTRL_ON GPIO_SetBits(GPIOH, GPIO_Pin_5)

#define LASER_OFF GPIO_ResetBits(GPIOG, GPIO_Pin_13)
#define POWER1_CTRL_OFF GPIO_ResetBits(GPIOH, GPIO_Pin_2)
#define POWER2_CTRL_OFF GPIO_ResetBits(GPIOH, GPIO_Pin_3)
#define POWER3_CTRL_OFF GPIO_ResetBits(GPIOH, GPIO_Pin_4)
#define POWER4_CTRL_OFF GPIO_ResetBits(GPIOH, GPIO_Pin_5)

void Power_Ctrl_Init(void);

#endif
