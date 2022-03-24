#include "listen.h"

#include "stdio.h"

/**
 * @brief 单个要监听的模块初始化
 * 
 * @param self 模块指针
 * @param cnt_num 超时倒数，超时时间=超时倒数*单次滴答时间
 * @param On_Hook 上线回调函数
 * @param Off_Hook 离线回调函数
 */
void Module_Status_Init(Module_status_t* self, u8 cnt_num, void On_Hook(void), void Off_Hook(void))
{
	self->count = 0;
	self->reload_cnt = cnt_num;
	
	self->time_out_flag = 1;
	self->old_time_out_flag = 1;
	
	self->Module_Online_Hook = On_Hook;
	self->Module_Offline_Hook = Off_Hook;
}

/**
 * @brief 定时执行该函数，以实现监听效果
 * 
 * @param self 模块指针
 */
void Module_Status_Listen(Module_status_t* self)
{
	self->old_time_out_flag = self->time_out_flag;
	
	if(self->count > 0)
	{
		self->count--;
		self->time_out_flag=0;
		if(self->Module_Online_Hook!=NULL && self->old_time_out_flag == 1)
		{
			self->Module_Online_Hook();
		}
	}
	else if(self->count == 0)
	{
		self->time_out_flag=1;
		if(self->Module_Offline_Hook!=NULL && self->old_time_out_flag == 0)
		{
			self->Module_Offline_Hook();
		}
	}
}

/**
 * @brief 模块收到数据时执行的函数
 * 
 * @param self 模块指针
 */
void Module_Status_Reload(Module_status_t* self)
{
	self->count = self->reload_cnt;
}

/**
 * @brief 获取单个模块状态
 * 
 * @param self 模块指针
 * @return uint8_t 模块在线---1u，模块离线---0u
 */
uint8_t Get_Module_State(const Module_status_t* self)
{
	return (self->time_out_flag == 0) ? 1u : 0u;
}
