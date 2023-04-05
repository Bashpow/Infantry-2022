#ifndef __GYROSCOPE_H_
#define __GYROSCOPE_H_

#include "stm32f4xx.h"

typedef struct
{
	struct
	{
		float roll_x;
		float pitch_y;
		float yaw_z;
	}angle;
}Gyro_Wit_Data_t;

#endif //__GYROSCOPE_H_
