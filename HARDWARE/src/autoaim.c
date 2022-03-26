#include "autoaim.h"
#include "usart6.h"

#include "pid.h"
#include "usart3.h"

/* 变量声明 */
static const uint8_t* autoaim_rx_buf;
static Auto_aim_t auto_aim_msg;

/**
 * @brief 自瞄系统初始化
 * 
 */
void Auto_Aim_Init(void)
{
	autoaim_rx_buf = Get_Usart6_Rx_Buf();
	auto_aim_msg.identifie_ready = 0;
	auto_aim_msg.x_yaw = 0;
	auto_aim_msg.y_pitch = 0;
	Usart6_Init();
}

/**
 * @brief 获取自瞄数据
 * 
 * @return const Auto_aim_t* 数据指针
 */
const Auto_aim_t* Get_Auto_Aim_Msg(void)
{
	return &auto_aim_msg;
}

/**
 * @brief 重置自瞄数据
 * 
 */
void Reset_Auto_Aim_Msg(void)
{
	auto_aim_msg.identifie_ready = 0;
	auto_aim_msg.x_yaw = 0;
	auto_aim_msg.y_pitch = 0;
	auto_aim_msg.target_color = 0;
}

/**
 * @brief 设置击打目标颜色
 * 
 * @param target_color 击打蓝色 0x0F0F，击打红色 0x1F1F
 */
void Set_Target_Color(uint16_t target_color)
{
	if(target_color == 0x0F0F)
	{
		uint8_t send_buffer[10] = {0x3F, 0x3F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x6C, 0x6C};
		Usart6_Send_Buf(send_buffer, 10);
	}
	else if(target_color == 0x1F1F)
	{
		uint8_t send_buffer[10] = {0x3F, 0x3F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x9C, 0x9C};
		Usart6_Send_Buf(send_buffer, 10);
	}
}

/**
 * @brief 解析自瞄数据
 * 
 * @return uint8_t 解析成功---1，解析失败---0
 */
uint8_t Auto_Aim_Calc(void)
{
#define AUTOAIM_RX_BUF_U16 ((uint16_t*)autoaim_rx_buf)
	/* 数据校验 */
	uint16_t check_sum_add = AUTOAIM_RX_BUF_U16[0]+AUTOAIM_RX_BUF_U16[1]+AUTOAIM_RX_BUF_U16[2]+AUTOAIM_RX_BUF_U16[3];
	uint16_t check_sum_end = AUTOAIM_RX_BUF_U16[4];
	if((check_sum_add != check_sum_end))
	{
		return 0;
	}
	
	/* 数据解析 */
	if(AUTOAIM_RX_BUF_U16[0] == 0x3F3F)
	{
		auto_aim_msg.identifie_ready = 1;
		auto_aim_msg.x_yaw = AUTOAIM_RX_BUF_U16[1];
		auto_aim_msg.y_pitch = AUTOAIM_RX_BUF_U16[2];
	}
	else if(AUTOAIM_RX_BUF_U16[0] == 0x2F2F)
	{
		auto_aim_msg.identifie_ready = 0;
	}
	auto_aim_msg.target_color = AUTOAIM_RX_BUF_U16[3];
	return 1;
#undef AUTOAIM_RX_BUF_U16
}

