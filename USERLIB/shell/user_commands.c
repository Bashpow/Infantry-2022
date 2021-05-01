#include "user_commands.h"
#include "shell.h"
#include "shellio.h"
extern Shell_command_t shell_cmd_root;

//用户头文件包含
#include "FreeRTOS.h"
#include "math2.h"
#include "detect_task.h"
#include "can1_motor.h"

//变量定义
static const Super_capacitor_t* super_cap_data;

//函数声明
static void Module_Online_Status(char * arg);
static void Super_Cap_Data(char * arg);
static void Pid_Show(char * arg);
static void Pid_Set(char * arg);


void User_Commands_Init(void)
{
	//变量初始化
	super_cap_data = Get_Super_Capacitor();

	//用户命令注册
	Shell_Register_Command("module-status" , Module_Online_Status);
	Shell_Register_Command("super-cap-data" , Super_Cap_Data);
	Shell_Register_Command("pid-show" , Pid_Show);
	Shell_Register_Command("pid-set" , Pid_Set);
}

/* -------------------------------------------------------------------- */
//row 行     columns 列
char** Create_Two_Dimensional_Array_Char(uint16_t rows, uint16_t columns)
{
	char **p = (char **)pvPortMalloc(sizeof(char *) * rows);
	for (int i = 0; i < rows; ++i)
	{
		p[i] = (char *)pvPortMalloc(sizeof(char) * columns);
	}
	return p;
}

void Delete_Two_Dimensional_Array_Char(char** arg, uint16_t rows)
{
	for (int i = 0; i < rows; ++i)
		vPortFree(arg[i]);
	vPortFree(arg);
}
/* -------------------------------------------------------------------- */

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
