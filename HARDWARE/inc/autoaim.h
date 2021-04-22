#ifndef __AUTOAIM_H
#define __AUTOAIM_H

#include "stm32f4xx.h"

#define AUTOAIM_X_YAW_CUR    460
#define AUTOAIM_Y_PITCH_CUR  240

typedef struct
{
	u8 identifie_ready; //识别成功， 2识别失败
	
	uint16_t x_yaw;
	uint16_t y_pitch;
}Auto_aim_t;

void Auto_Aim_Init(void);
const Auto_aim_t* Get_Auto_Aim_Msg(void);
void Reset_Auto_Aim_Msg(void);
void Auto_Aim_Calc(void);

#endif
