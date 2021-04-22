#include "motor.h"

void Calculate_Motor_Data(Motor_measure_t* motor, CanRxMsg* can_message)
{
	motor->mechanical_angle = (uint16_t)((can_message)->Data[0] << 8 | (can_message)->Data[1]);
	motor->speed_rpm = (uint16_t)((can_message)->Data[2] << 8 | (can_message)->Data[3]);
	motor->actual_torque_current = (uint16_t)((can_message)->Data[4] << 8 | (can_message)->Data[5]);
	motor->temperate = (can_message)->Data[6];
}

