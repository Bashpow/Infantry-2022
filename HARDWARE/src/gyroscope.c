#include "gyroscope.h"

/**
 * @brief 检测陀螺仪数据是否合法
 * 
 * @param gyro_raw_buf 原始数据，长度为10Byte
 * @return uint8_t 校验成功---1，校验失败---0
 */
uint8_t Check_Gyro_Data_Available(const uint8_t* gyro_raw_buf)
{
	uint8_t sum_add = 0;
	if(gyro_raw_buf[0] != 0x55) { return 0; }
	for(uint8_t i=0; i<10; i++) {
		sum_add += gyro_raw_buf[i];
	}
	if(gyro_raw_buf[10] != sum_add) { return 0; }
	return 1;
}

/**
 * @brief 解析原始数据，使用前应确保数据合法
 * 
 * @param gyro_raw_buf 原始数据
 * @param gyro_data 数据存放位置
 */
void Parse_Gyro_Data(const uint8_t* gyro_raw_buf, Gyro_Wit_Data_t* gyro_data) {
    // 因为使用的是单轴陀螺仪，故只解析yaw轴
	gyro_data->angle.roll_x = ((short)(gyro_raw_buf[3]<<8|gyro_raw_buf[2]))/32768.0*180;
	gyro_data->angle.pitch_y = (gyro_raw_buf[5]<<8|gyro_raw_buf[4]))/32768.0*180;
    gyro_data->angle.yaw_z = ((short)(gyro_raw_buf[7] << 8 | gyro_raw_buf[6])) / 32768.0 * 180;
    if (gyro_data->angle.yaw_z < 0) {
        gyro_data->angle.yaw_z += 360;
    }
}
