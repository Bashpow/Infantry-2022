#include "remoter_task.h"

#include "remoter.h"
#include "usart1.h"
#include "usart3.h"
#include "judge_system.h"
#include "buzzer_task.h"
#include "shooter_task.h"
#include "detect_task.h"

#define S1_VALUE       remote_controller.rc.s1
#define S2_VALUE       remote_controller.rc.s2
#define OLD_S1_VALUE   last_time_rc.rc.s1
#define OLD_S2_VALUE   last_time_rc.rc.s2
#define KEY_VALUE      remote_controller.key.value
#define OLD_KEY_VALUE  last_time_rc.key.value

#define S1_AT(a)       (S1_VALUE == (a))
#define S2_AT(a)       (S2_VALUE == (a))

#define S1_CHANGED_TO(a,b) ((OLD_S1_VALUE == (a)) && (S1_VALUE == (b)))
#define S2_CHANGED_TO(a,b) ((OLD_S2_VALUE == (a)) && (S2_VALUE == (b)))

#define KEY_CLICKED(key)   (RC_KEY_PRESSED(KEY_VALUE,key) && (!RC_KEY_PRESSED(OLD_KEY_VALUE,key)))

/* 函数声明 */
static void Robot_Rc_Mode_Change_Control(void);
static void Shoot_Key_Control(void);
void Switch_Mouse_Key_Change(Rc_ctrl_t* rc, Rc_ctrl_t* last_rc, Robot_mode_t* robot_mode);

/* 变量 */
TaskHandle_t RemoterTask_Handler;
static Rc_ctrl_t remote_controller;  //本次获取的遥控器数据
static Rc_ctrl_t last_time_rc;  //上一次的遥控器数据
static Robot_mode_t robot_mode;  //机器人模式
static const uint8_t* rc_rx_buf[2];  //串口1接收原始数据数组指针
static SemaphoreHandle_t rc_data_update_semaphore;  //串口1DMA接收信号量
static const Judge_data_t* judge_data;

void Remoter_Task(void *pvParameters)
{
	static uint8_t rx_available_bufx;

	//二值信号量初始化
	rc_data_update_semaphore = xSemaphoreCreateBinary();
	
	//取得遥控器DBUS原始数据指针
	rc_rx_buf[0] = Get_Rc_Bufferx(0);
	rc_rx_buf[1] = Get_Rc_Bufferx(1);
	
	judge_data = Get_Judge_Data();

	//重置遥控器数据
	Rc_Data_Reset(&remote_controller);
	Rc_Data_Reset(&last_time_rc);
	
	//初始化机器人模式
	{
		robot_mode.control_device=1;  //操控设备选择 1 键鼠  2遥控器
		robot_mode.mouse_key_chassis_mode=1; //1跟随 2小陀螺 3特殊
		robot_mode.mouse_key_gimbal_mode=1; //1手动 2自瞄 3特殊
		robot_mode.fric_cover_mode=0; //摩擦轮模式 0关闭 1慢速 2快速 3开舱盖
		robot_mode.shoot_mode=1; //射击模式 1单发 2三连发 3连发速度一 4连发速度二
		robot_mode.motion_mode=5; //1底盘跟随 2小陀螺 3自瞄跟随 4自瞄小陀螺 5特殊
		robot_mode.shoot_key=0; //射击开关 1单发 2三连发 3连发速度一 4连发速度二
	}
	
	vTaskDelay(200);
	
	//开启串口1DMA接收完成中断
	DMA_ITConfig(DMA2_Stream2, DMA_IT_TC, ENABLE);

	while(1)
	{
		
		//等待信号量，超时时间50ms
		if( xSemaphoreTake(rc_data_update_semaphore, 500) == pdTRUE )
		{
			/* 获取当前遥控器可用原始数据 */
			rx_available_bufx = Get_Rc_Available_Bufferx();

			/* 解析遥控器数据 */
			Parse_Remoter_Data(rc_rx_buf[rx_available_bufx], &remote_controller);

			/* 检测遥控器数据，是否合法如果不合法采取操作 */
			if(Remoter_Data_Check(&remote_controller))
			{
				//保持当前遥控器数据不变
				Rc_Data_Copy(&remote_controller, &last_time_rc);
				//重置串口1及DMA
				Usart1_DMA_Reset();
				
				DEBUG_ERROR(100);
			}
			
			/* 更新遥控器状态 */
			Detect_Reload(0);
			
			/* 机器人模式变换响应 */
			Robot_Rc_Mode_Change_Control();
			
			/* 响应键盘控制 */
			Switch_Mouse_Key_Change(&remote_controller, &last_time_rc, &robot_mode);

			/* 射击按钮 */
			Shoot_Key_Control();
			
			/* 保存本次遥控器状态 */
			Rc_Data_Copy(&last_time_rc, &remote_controller);
			
		}
		
		//若等待信号量超时
		else
		{
			Rc_Data_Reset(&remote_controller);
			Rc_Data_Reset(&last_time_rc);
		}
		
	}
	
	//vTaskDelete(NULL);
}

/*
  函数名：Rc_Data_Update
	描述  ：在遥控器串口DMA接收函数时调用，用于通知此任务（remoter task）遥控器数据接收完毕
  参数  ：无
  返回值：无
*/
void Rc_Data_Update(void)
{
	BaseType_t pxHigherPriorityTaskWoken;
	// 释放二值信号量，发送接收到新数据标志，供前台程序查询
	xSemaphoreGiveFromISR(rc_data_update_semaphore, &pxHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

/*
  函数名：Clear_Shoot_Key
	描述  ：在必要时清除射击按钮值
  参数  ：无
  返回值：无
*/
void Clear_Shoot_Key(void)
{
	robot_mode.shoot_key = 0;
}

void Fric_Reset(void)
{
	robot_mode.fric_cover_mode = 0;
}

/*
  函数名：Get_Remote_Control_Point
	描述  ：获取遥控器数据
  参数  ：无
  返回值：Rc_ctrl_t结构体变量
*/
const Rc_ctrl_t *Get_Remote_Control_Point(void)
{
    return &remote_controller;
}

/*
  函数名：Get_Robot_Mode_Point
	描述  ：获取机器人模式数据
  参数  ：无
  返回值：Robot_mode_t结构体变量
*/
const Robot_mode_t *Get_Robot_Mode_Point(void)
{
    return &robot_mode;
}

/* 响应遥控器按键切换模式 */
static void Robot_Rc_Mode_Change_Control(void)
{
	/* 操控设备选择 */
	if(S2_CHANGED_TO(3,2))
	{
		robot_mode.control_device++;
		if(robot_mode.control_device==3) robot_mode.control_device=1;
		Set_Beep_Time(robot_mode.control_device, 1000, 55);
	}
	
	/* 底盘云台模式 */
	if(S2_CHANGED_TO(3,1))
	{
		robot_mode.motion_mode++;
		if(robot_mode.motion_mode==6) robot_mode.motion_mode=1;
		Set_Beep_Time(robot_mode.motion_mode, 1200, 50);
	}

	if(robot_mode.control_device != 2)
	{
		return;
	}

	/* 摩擦轮速度模式，舱盖开关 */
	if(S1_CHANGED_TO(3,2))
	{
		robot_mode.fric_cover_mode++;
		if(robot_mode.fric_cover_mode==4) robot_mode.fric_cover_mode=0;
		Set_Beep_Time(robot_mode.fric_cover_mode+1, 800, 60);
	}

	/* 射击模式选择器 */
	if(S1_CHANGED_TO(3,1))
	{
		robot_mode.shoot_mode++;
		if(robot_mode.shoot_mode==5) robot_mode.shoot_mode=1;
		Set_Beep_Time(robot_mode.shoot_mode, 600, 65);
	}
}

/* 响应键盘切换模式 */
void Switch_Mouse_Key_Change(Rc_ctrl_t* rc, Rc_ctrl_t* last_rc, Robot_mode_t* robot_mode)
{
	//判断是不是键鼠模式
	if(robot_mode->control_device != 1)
	{
		return;
	}

	//底盘模式(按住shift开启小陀螺)
	if( RC_KEY_PRESSED(KEY_VALUE,KEY_SHIFT) )
	{
		robot_mode->mouse_key_chassis_mode = 2;
	}
	else
	{
		if(robot_mode->mouse_key_chassis_mode != 3)
			robot_mode->mouse_key_chassis_mode = 1;
	}

	//云台模式
	if(KEY_CLICKED(KEY_CTRL))
	{
		if(robot_mode->mouse_key_gimbal_mode == 1)
		{
			robot_mode->mouse_key_gimbal_mode = 2;
			Set_Beep_Time(4, 1200, 50);
		}
		else
		{
			robot_mode->mouse_key_gimbal_mode = 1;
			Set_Beep_Time(3, 1200, 50);
		}
	}

	//特殊模式
	if( KEY_CLICKED(KEY_Z) )
	{
		robot_mode->mouse_key_chassis_mode = 3;
		robot_mode->mouse_key_gimbal_mode = 3;
		Set_Beep_Time(5, 1200, 50);
	}

	//特殊模式检查
	if(robot_mode->mouse_key_chassis_mode == 3 && robot_mode->mouse_key_gimbal_mode != 3)
	{
		robot_mode->mouse_key_chassis_mode = 1;
	}
	if(robot_mode->mouse_key_chassis_mode != 3 && robot_mode->mouse_key_gimbal_mode == 3)
	{
		robot_mode->mouse_key_gimbal_mode = 1;
	}

	//弹舱模式
	if(KEY_CLICKED(KEY_Q))
	{
		if(robot_mode->fric_cover_mode == 3)
		{
			robot_mode->fric_cover_mode = 0;
			Set_Beep_Time(1, 800, 60);
		}
		else
		{
			robot_mode->fric_cover_mode = 3;
			Set_Beep_Time(4, 800, 60);
		}
	}

	//摩擦轮模式
	if(KEY_CLICKED(KEY_E))
	{
		if(robot_mode->fric_cover_mode == 1)
		{
			robot_mode->fric_cover_mode = 0;
			Set_Beep_Time(1, 800, 60);
		}
		else
		{
			robot_mode->fric_cover_mode = 1;
			Set_Beep_Time(2, 800, 60);
		}
	}
}

/* 响应射击模式 */
static void Shoot_Key_Control(void)
{
	static u8 mouse_first_press = 1;

	//遥控器模式
	if(robot_mode.control_device == 2)
	{
		/* 射击按钮 */
		if (robot_mode.shoot_mode == 1 || robot_mode.shoot_mode == 2)
		{
			if (last_time_rc.rc.ch4 < 120 && remote_controller.rc.ch4 > 120)
			{
				robot_mode.shoot_key = robot_mode.shoot_mode;
			}
		}
		else
		{
			if (remote_controller.rc.ch4 > 120)
			{
				robot_mode.shoot_key = robot_mode.shoot_mode;
			}
			else
			{
				robot_mode.shoot_key = 0;
			}
		}
	}
	
	//键盘模式
	else
	{
		if( Is_Id1_17mm_Excess_Heat(judge_data) == 0 )  //判断是否超热量
		{
			if(remote_controller.mouse.press_l)
			{
				if(mouse_first_press)
				{
					First_Shoot_Friction_Speed_Subtract(2);
					mouse_first_press = 0;
				}
				robot_mode.shoot_mode = 3;
				robot_mode.shoot_key = 3;
			}
			else
			{
				mouse_first_press = 1;
				robot_mode.shoot_key = 0;
			}

			if( remote_controller.mouse.press_r && (!last_time_rc.mouse.press_r) )
			{
				First_Shoot_Friction_Speed_Subtract(2);
				robot_mode.shoot_mode = 2;
				robot_mode.shoot_key = 2;
			}
		}
		else
		{
			robot_mode.shoot_key = 0;
		}

	}

}

