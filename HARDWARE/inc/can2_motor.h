#ifndef __CAN2_MOTOR_H
#define __CAN2_MOTOR_H

#include "stm32f4xx.h"

#include "motor.h"

#define PITCH_UP_LIMIT 6960
#define PITCH_MID_ANGLE 7490
#define PITCH_DOWN_LIMIT 8010

void Set_Shooter_Wave_Motors_Speed(float wave_wheel);
void Set_Gimbal_Motors_Speed(float speed_yaw, float speed_pitch);

float Calc_Yaw_Angle360_Pid(float tar_angle, float cur_angle);
float Calc_Pitch_Angle8191_Pid(float tar_angle);
const Motor_measure_t *Get_Gimbal_Motor(void);
const Motor_measure_t *Get_Shooter_Wave_Motor(void);

#endif
