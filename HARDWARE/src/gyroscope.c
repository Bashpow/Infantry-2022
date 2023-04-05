#include "gyroscope.h"

/**
 * @brief ��������������Ƿ�Ϸ�
 * 
 * @param gyro_raw_buf ԭʼ���ݣ�����Ϊ10Byte
 * @return uint8_t У��ɹ�---1��У��ʧ��---0
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
 * @brief ����ԭʼ���ݣ�ʹ��ǰӦȷ�����ݺϷ�
 * 
 * @param gyro_raw_buf ԭʼ����
 * @param gyro_data ���ݴ��λ��
 */
void Parse_Gyro_Data(const uint8_t* gyro_raw_buf, Gyro_Wit_Data_t* gyro_data) {
    // ��Ϊʹ�õ��ǵ��������ǣ���ֻ����yaw��
	gyro_data->angle.roll_x = ((short)(gyro_raw_buf[3]<<8|gyro_raw_buf[2]))/32768.0*180;
	gyro_data->angle.pitch_y = (gyro_raw_buf[5]<<8|gyro_raw_buf[4]))/32768.0*180;
    gyro_data->angle.yaw_z = ((short)(gyro_raw_buf[7] << 8 | gyro_raw_buf[6])) / 32768.0 * 180;
    if (gyro_data->angle.yaw_z < 0) {
        gyro_data->angle.yaw_z += 360;
    }
}
