#include "user_commands.h"
#include "shell.h"
#include "shellio.h"
extern Shell_command_t shell_cmd_root;

//用户头文件包含
#include "math2.h"
#include "detect_task.h"
#include "can1_motor.h"
#include "can2_motor.h"
#include "wt61c_task.h"
#include "autoaim.h"

//变量定义
static const Motor_measure_t* chassis_motor;
static const Motor_measure_t* shooter_wave_motor; //波轮电机数据
static const Motor_measure_t* gimbal_motor; //云台电机数据
static const Auto_aim_t* auto_aim_msg;
static const Super_capacitor_t* super_cap_data;
static const Wt61c_Data_t* wt61c_data;

//函数声明
static void Module_Online_Status(char * arg);
static void Motors_Data(char * arg);
static void Autoaim_Data(char * arg);
static void Super_Cap_Data(char * arg);
static void Gyroscope_Data(char * arg);
static void Pid_Show(char * arg);
static void Pid_Set(char * arg);
static void Pid2_Show(char * arg);
static void Pid2_Set(char * arg);

//用户命令初始化
void User_Commands_Init(void)
{
	//变量初始化
	chassis_motor = Get_Chassis_Motor();
	shooter_wave_motor = Get_Shooter_Wave_Motor();
	gimbal_motor = Get_Gimbal_Motor();
	auto_aim_msg = Get_Auto_Aim_Msg();
	super_cap_data = Get_Super_Capacitor();
	wt61c_data = Get_Wt61c_Data();

	//用户命令注册
	Shell_Register_Command("module-status" , Module_Online_Status);
	Shell_Register_Command("motors-data" , Motors_Data);
	Shell_Register_Command("word.exe" , Autoaim_Data);
	Shell_Register_Command("super-cap-data" , Super_Cap_Data);
	Shell_Register_Command("gyroscope-data" , Gyroscope_Data);
	Shell_Register_Command("pid-show" , Pid_Show);
	Shell_Register_Command("pid-set" , Pid_Set);
	Shell_Register_Command("pid2-show" , Pid2_Show);
	Shell_Register_Command("pid2-set" , Pid2_Set);
}

#define ONLINE_STATUS_PRINT(module) { if(Get_Module_Online_State(module)){shell_print("ON-line\r\n");}else{shell_print("OFF-line\r\n");} }
static void Module_Online_Status(char * arg)
{
	shell_print("Module----------\r\n");

	shell_print("remote control:\t");
	ONLINE_STATUS_PRINT(0);

	shell_print("chassis motor:\t");
	ONLINE_STATUS_PRINT(1);

	shell_print("gimbal motor:\t");
	ONLINE_STATUS_PRINT(2);

	shell_print("shooter motor:\t");
	ONLINE_STATUS_PRINT(3);

	shell_print("auto aim:\t");
	ONLINE_STATUS_PRINT(4);

	shell_print("judge system:\t");
	ONLINE_STATUS_PRINT(5);

	shell_print("super cap:\t");
	ONLINE_STATUS_PRINT(6);

	shell_print("gyroscope:\t");
	ONLINE_STATUS_PRINT(7);

	shell_print("----------------\r\n");
}

#define PRINT_MOTOR_C620_DATA(name, data) shell_print("%s\tangle: %d, speed: %drpm, current: %.1fA, temperate: %dC\r\n", name, data.mechanical_angle, data.speed_rpm, (((float)(data.actual_torque_current))*20.0f/16384.0f), data.temperate);
#define PRINT_MOTOR_GM6020_DATA(name, data) shell_print("%s\tangle: %d, speed: %drpm, current: %d, temperate: %dC\r\n", name, data.mechanical_angle, data.speed_rpm, data.actual_torque_current, data.temperate);
#define PRINT_MOTOR_C615_DATA(name, data) shell_print("%s\tangle: %d, speed: %drpm, torque: %d\r\n", name, data.mechanical_angle, data.speed_rpm, data.actual_torque_current);
static void Motors_Data(char * arg)
{
	PRINT_MOTOR_C620_DATA("chassis motor1", chassis_motor[0]);
	PRINT_MOTOR_C620_DATA("chassis motor2", chassis_motor[1]);
	PRINT_MOTOR_C620_DATA("chassis motor3", chassis_motor[2]);
	PRINT_MOTOR_C620_DATA("chassis motor4", chassis_motor[3]);
	PRINT_MOTOR_GM6020_DATA("yaw motor", gimbal_motor[0]);
	PRINT_MOTOR_GM6020_DATA("pitch motor", gimbal_motor[1]);
	PRINT_MOTOR_C615_DATA("wave motor", shooter_wave_motor[0]);
	shell_print("\r\n");
}

static void Autoaim_Data(char * arg)
{
	if(Get_Module_Online_State(4))
	{
		if(auto_aim_msg->identifie_ready)
		{
			shell_print("Target found!\tx:%d, y:%d", auto_aim_msg->x_yaw, auto_aim_msg->y_pitch);
		}
		else
		{
			shell_print("Target not found!");
		}
	}
	else
	{
		shell_print("Nuc offline!");
	}
	shell_print("\r\n");
}

static void Super_Cap_Data(char * arg)
{
	if(Get_Module_Online_State(6))
	{
		shell_print("input voltage:%.2f, cap voltage:%.2f, input current:%.2f, target power:%.2f", super_cap_data->input_voltage, super_cap_data->cap_voltage, super_cap_data->input_current, super_cap_data->target_power);
	}
	else
	{
		shell_print("Super capacitor offline!");
	}
	shell_print("\r\n");
}

static void Gyroscope_Data(char * arg)
{
	if(Get_Module_Online_State(7))
	{
		shell_print("roll:%.2f, pitch:%.2f, yaw:%.2f", wt61c_data->angle.roll_x, wt61c_data->angle.pitch_y, wt61c_data->angle.yaw_z);
	}
	else
	{
		shell_print("Gyroscope offline!");
	}
	shell_print("\r\n");
}

float easy_pid_p, easy_pid_i, easy_pid_d;
static void Pid_Show(char * arg)
{
	shell_print("p:%.6f\ti:%.6f\td:%.6f\r\n", easy_pid_p, easy_pid_i, easy_pid_d);
}
static void Pid_Set(char * arg)
{
	/*
	char * argv[4];
	int argc = Shell_Split_String((char*)arg,argv,4);
	shell_print("get %d parameter\r\n", argc);
	for(int i = 0 ; i < argc ; ++i)
		shell_print("argv[%d]: \"%s\"\r\n",i,argv[i]);
	*/
	char * argv[3];
	int argc = Shell_Split_String(arg, argv, 3);
	switch(argv[1][0])
	{
		case 'p':
			easy_pid_p = String_To_Float(argv[2]);
			shell_print("EASY PID p set %.6f", easy_pid_p);
			break;

		case 'i':
			easy_pid_i = String_To_Float(argv[2]);
			shell_print("EASY PID i set %.6f", easy_pid_i);
			break;

		case 'd':
			easy_pid_d = String_To_Float(argv[2]);
			shell_print("EASY PID d set %.6f", easy_pid_d);
			break;

		default:
			shell_print("Error!");
			break;
	}
	shell_print("\r\n");
}

float easy_pid2_p, easy_pid2_i, easy_pid2_d;
static void Pid2_Show(char * arg)
{
	shell_print("p:%.6f\ti:%.6f\td:%.6f\r\n", easy_pid2_p, easy_pid2_i, easy_pid2_d);
}
static void Pid2_Set(char * arg)
{
	char * argv[3];
	int argc = Shell_Split_String(arg, argv, 3);
	switch(argv[1][0])
	{
		case 'p':
			easy_pid2_p = String_To_Float(argv[2]);
			shell_print("EASY PID2 p set %.6f", easy_pid2_p);
			break;

		case 'i':
			easy_pid2_i = String_To_Float(argv[2]);
			shell_print("EASY PID2 i set %.6f", easy_pid2_i);
			break;

		case 'd':
			easy_pid2_d = String_To_Float(argv[2]);
			shell_print("EASY PID2 d set %.6f", easy_pid2_d);
			break;

		default:
			shell_print("Error!");
			break;
	}
	shell_print("\r\n");
}
