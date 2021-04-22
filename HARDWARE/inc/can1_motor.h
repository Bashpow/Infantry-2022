#ifndef __CAN1_MOTOR_H
#define __CAN1_MOTOR_H

#include "motor.h"

typedef struct
{
	float input_voltage; //输入电压
	float cap_voltage; //电容电压
	float input_current; //输入电流
	float target_power; //目标功率
	
}Super_capacitor_t;

const Motor_measure_t *Get_Chassis_Motor(void);
const Super_capacitor_t *Get_Super_Capacitor(void);
void Set_Chassis_Motors_Speed(float speed_fl, float speed_fr, float speed_bl, float speed_br);
void Set_Super_Capacitor(uint16_t target_power);

#endif
