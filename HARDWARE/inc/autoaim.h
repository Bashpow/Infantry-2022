#ifndef __AUTOAIM_H
#define __AUTOAIM_H

#include "stm32f4xx.h"

#define AUTOAIM_X_YAW_CUR    460
#define AUTOAIM_Y_PITCH_CUR  240

typedef struct
{
	uint8_t identifie_ready; //识别成功， 2识别失败
	uint16_t x_yaw;
	uint16_t y_pitch;
	uint16_t target_color;
}Auto_aim_t;

void Auto_Aim_Init(void);
const Auto_aim_t* Get_Auto_Aim_Msg(void);
void Reset_Auto_Aim_Msg(void);
void Set_Target_Color(uint16_t target_color);
uint8_t Auto_Aim_Calc(void);

#endif
