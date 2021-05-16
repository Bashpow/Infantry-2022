#include "can1_motor.h"
#include "can2_motor.h"

#include "stm32f4xx.h"
#include "detect_task.h"
#include "can1.h"
#include "pid.h"
#include "cansend_task.h"

/* ------------------重要说明--------------------- */
//CAN1连接了地盘4个电机，和超级电容
//规范文件名应为can1_motor.h --> can1_device.h
/* ---------------------------------------------- */

//变量定义
static Motor_measure_t chassis_motor[4];
static Super_capacitor_t super_capacitor;

//函数声明
static Pid_Position_t motor_fl_speed_pid = NEW_POSITION_PID(11, 0, 5, 2000, 16000, 0, 1000, 500);
static Pid_Position_t motor_fr_speed_pid = NEW_POSITION_PID(11, 0, 5, 2000, 16000, 0, 1000, 500);
static Pid_Position_t motor_bl_speed_pid = NEW_POSITION_PID(11, 0, 5, 2000, 16000, 0, 1000, 500);
static Pid_Position_t motor_br_speed_pid = NEW_POSITION_PID(11, 0, 5, 2000, 16000, 0, 1000, 500);
static void Can1_Hook(CanRxMsg *rx_message);  //M3508减速箱减速比约为19：1  max8740

/* CAN1 send and receive ID */
typedef enum
{
	CAN_CHASSIS_ALL_ID = 0x200,
	CAN_3508_M1_ID = 0x201,
	CAN_3508_M2_ID = 0x202,
	CAN_3508_M3_ID = 0x203,
	CAN_3508_M4_ID = 0x204,
	
	SUPER_CAPACITOR_ID = 0x211,
	SUPER_CAPACITOR_SEND_ID = 0x210,
	
} can1_msg_id_e;

//Can1接收中断
void CAN1_RX0_IRQHandler(void)
{
	static CanRxMsg rx1_message;
	
	if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET)
	{
		CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
		CAN_Receive(CAN1, CAN_FIFO0, &rx1_message);
		Can1_Hook(&rx1_message);
	}
}

const Motor_measure_t *Get_Chassis_Motor(void)
{
    return chassis_motor;
}

const Super_capacitor_t *Get_Super_Capacitor(void)
{
	return &super_capacitor;
}

//计算底盘4个电机速度PID，并输出
void Set_Chassis_Motors_Speed(float speed_fl, float speed_fr, float speed_bl, float speed_br)
{
	Can_Send(1,
			 CAN_CHASSIS_ALL_ID,
			 Pid_Position_Calc(&motor_fl_speed_pid, speed_fl, chassis_motor[0].speed_rpm),
			 Pid_Position_Calc(&motor_fr_speed_pid, speed_fr, chassis_motor[1].speed_rpm),
			 Pid_Position_Calc(&motor_bl_speed_pid, speed_bl, chassis_motor[2].speed_rpm),
			 Pid_Position_Calc(&motor_br_speed_pid, speed_br, chassis_motor[3].speed_rpm));
}


//超级电容功率设置
void Set_Super_Capacitor(uint16_t target_power)
{
	Can_Send(1, SUPER_CAPACITOR_SEND_ID, target_power, 0, 0, 0);
}

//超级电容数据处理
static void Calculate_Super_Capacitor(Super_capacitor_t* cap_data, CanRxMsg* can_message)
{
	Detect_Reload(6);  //刷新超级电容状态
	
	cap_data->input_voltage = (float)((can_message)->Data[1] << 8 | (can_message)->Data[0]) / 100.f;
	cap_data->cap_voltage = (float)((can_message)->Data[3] << 8 | (can_message)->Data[2]) / 100.f;
	cap_data->input_current = (float)((can_message)->Data[5] << 8 | (can_message)->Data[4]) / 100.f;
	cap_data->target_power = (float)((can_message)->Data[7] << 8 | (can_message)->Data[6]) / 100.f;
}

/* 统一处理can中断函数，并且记录发送数据的时间，作为离线判断依据 */
static void Can1_Hook(CanRxMsg *rx_message)
{
	switch (rx_message->StdId)
	{
		case CAN_3508_M1_ID:
		case CAN_3508_M2_ID:
		case CAN_3508_M3_ID:
		case CAN_3508_M4_ID:
		{
			uint8_t i = 0;
			//处理电机ID号
			i = rx_message->StdId - CAN_3508_M1_ID;
			//处理电机数据宏函数
			Calculate_Motor_Data(&chassis_motor[i], rx_message);
			//记录时间
			Classis_Reload(i);
			break;
		}
		
		
		case SUPER_CAPACITOR_ID:
		{
			Calculate_Super_Capacitor(&super_capacitor, rx_message);
			//DEBUG_SHOWDATA2("voltage:", super_capacitor.cap_voltage);
			//DEBUG_SHOWDATA2("gl", super_capacitor.target_power);
			
			break;
		}
		
		
		default:
		{
			break;
		}
		
	}
}

