#include "autoaim.h"
#include "usart6.h"

#include "pid.h"
#include "usart3.h"

/* 变量声明 */
static const uint8_t* autoaim_rx_buf;
static Auto_aim_t auto_aim_msg;

//自瞄系统初始化
void Auto_Aim_Init(void)
{
	autoaim_rx_buf = Get_Usart6_Rx_Buf();
	auto_aim_msg.identifie_ready = 0;
	auto_aim_msg.x_yaw = 0;
	auto_aim_msg.y_pitch = 0;
	Usart6_Init();
}

//获取自瞄数据指针
const Auto_aim_t* Get_Auto_Aim_Msg(void)
{
	return &auto_aim_msg;
}

//重置自瞄数据
void Reset_Auto_Aim_Msg(void)
{
	auto_aim_msg.identifie_ready = 0;
	auto_aim_msg.x_yaw = 0;
	auto_aim_msg.y_pitch = 0;
}

//自瞄数据计算
void Auto_Aim_Calc(void)
{	
	/* 数据校验 */
	uint16_t check_sum_add = ((autoaim_rx_buf[0]|autoaim_rx_buf[1]<<8)+(autoaim_rx_buf[2]|autoaim_rx_buf[3]<<8)+(autoaim_rx_buf[4]|autoaim_rx_buf[5]<<8)+(autoaim_rx_buf[6]|autoaim_rx_buf[7]<<8));
	uint16_t check_sum_end = (autoaim_rx_buf[8] | autoaim_rx_buf[9]<<8);
	
	if((check_sum_add != check_sum_end))
	{
		Usart6_DMA_Reset();
		DEBUG_ERROR(601);
		return;
	}
	
	/* 数据解析 */
	if(autoaim_rx_buf[0] == 0x3F && autoaim_rx_buf[1] == 0x3F)
	{
		auto_aim_msg.identifie_ready = 1;
		auto_aim_msg.x_yaw = (autoaim_rx_buf[3]<<8|autoaim_rx_buf[2]);
		auto_aim_msg.y_pitch = (autoaim_rx_buf[5]<<8 | autoaim_rx_buf[4]);
		//TODO:卡尔曼滤波
	}
	else if(autoaim_rx_buf[0] == 0x2F && autoaim_rx_buf[1] == 0x2F && check_sum_end ==0xBCBC)
	{
		auto_aim_msg.identifie_ready = 0;
	}

//	 DEBUG_PRINT("r:%d  x:%d  y:%d\r\n", auto_aim_msg.identifie_ready, auto_aim_msg.x_yaw, auto_aim_msg.y_pitch);

}

