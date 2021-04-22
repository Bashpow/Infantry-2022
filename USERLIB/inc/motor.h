#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f4xx.h"
#include "pid.h"

typedef struct
{
	uint16_t mechanical_angle;  //ת�ӻ�е�Ƕ�
	int16_t speed_rpm;  //ת��ת��
	int16_t actual_torque_current;  //ʵ��ת�ص���
	uint8_t temperate;  //����¶�
}Motor_measure_t;

void Calculate_Motor_Data(Motor_measure_t* motor, CanRxMsg* can_message);

#endif
