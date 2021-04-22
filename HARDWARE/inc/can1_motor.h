#ifndef __CAN1_MOTOR_H
#define __CAN1_MOTOR_H

#include "motor.h"

typedef struct
{
	float input_voltage; //�����ѹ
	float cap_voltage; //���ݵ�ѹ
	float input_current; //�������
	float target_power; //Ŀ�깦��
	
}Super_capacitor_t;

const Motor_measure_t *Get_Chassis_Motor(void);
const Super_capacitor_t *Get_Super_Capacitor(void);
void Set_Chassis_Motors_Speed(float speed_fl, float speed_fr, float speed_bl, float speed_br);
void Set_Super_Capacitor(uint16_t target_power);

#endif
