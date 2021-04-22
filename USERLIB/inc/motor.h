#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f4xx.h"
#include "pid.h"

typedef struct
{
	uint16_t mechanical_angle;  //转子机械角度
	int16_t speed_rpm;  //转子转速
	int16_t actual_torque_current;  //实际转矩电流
	uint8_t temperate;  //电机温度
}Motor_measure_t;

void Calculate_Motor_Data(Motor_measure_t* motor, CanRxMsg* can_message);

#endif
