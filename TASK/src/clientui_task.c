#include "clientui_task.h"
#include "client_ui.h"
#include "usart8.h"
#include "judge_system.h"
#include "can1_motor.h"
#include "detect_task.h"

static uint8_t draw_status = 0;

// 发送缓冲区
static uint8_t client_ui_send_buffer[128];

// 静态字符、图像
static Graph_Data static_graph;
// static String_Data static_char;

// 动态字符、图像
static Graph_Data cap_voltage_float; //超级电容电压
static Graph_Data cap_remaining_percentage; //超级电容剩余百分比 

TaskHandle_t ClientuiTask_Handler;

#define UI_SEND_DELAY_TIME_MS 500
#define CLIENT_UI_SEND_BUFFER() {while(0 == Uart8_Transmit_Start((uint32_t)&client_ui_send_buffer[0], UI_Get_Send_Data_Len(client_ui_send_buffer))){vTaskDelay(UI_SEND_DELAY_TIME_MS);}vTaskDelay(UI_SEND_DELAY_TIME_MS);}

static void Front_Sight_Draw(void);

void Clientui_Task(void *pvParameters)
{
	vTaskDelay(1000);

	// 设置对应的机器人ID
	// 判断裁判系统是否在线
	for(;;) {
WAIT_UI_RUN:
		vTaskDelay(300);
		UI_Set_Comparable_Id(Get_Judge_Data()->ext_game_robot_status_t.robot_id);
		if(Get_Module_Online_State(5) && draw_status) { //TODO:如果裁判系统在线且按下了按键{}
			break;
		}
	}
	
	// 清除所有
	UI_Delete(client_ui_send_buffer, UI_Data_Del_ALL, 0);
	CLIENT_UI_SEND_BUFFER();
	vTaskDelay(UI_SEND_DELAY_TIME_MS);

	// CSGO十字准星
	Front_Sight_Draw();

	// 超级电容电压、剩余百分比
	Float_Draw(&cap_voltage_float, "cpv", UI_Graph_ADD, 0, UI_Color_Green, 16, 2, 2, 1200, 300, 0.0f);
	Float_Draw(&cap_remaining_percentage, "cpp", UI_Graph_ADD, 0, UI_Color_Green, 16, 2, 2, 1200, 270, 0.0f);
	UI_ReFresh(client_ui_send_buffer, 2, cap_voltage_float, cap_remaining_percentage);
	CLIENT_UI_SEND_BUFFER();

	vTaskDelay(200);

	for(;;)
	{
		// 超级电容电压、剩余百分比
		Float_Draw(&cap_voltage_float, "cpv", UI_Graph_Change, 0, UI_Color_Green, 15, 2, 2, 1200, 300, Get_Super_Capacitor()->cap_voltage);
		Float_Draw(&cap_remaining_percentage, "cpp", UI_Graph_Change, 0, UI_Color_Green, 16, 2, 2, 1200, 270,
					(Get_Super_Capacitor()->cap_voltage>12.0f) ? ((Get_Super_Capacitor()->cap_voltage-12.0f)*100.0f/(Get_Super_Capacitor()->input_voltage-12.0f)):0.0f);
		UI_ReFresh(client_ui_send_buffer, 2, cap_voltage_float, cap_remaining_percentage);
		if(!draw_status){
			UI_Delete(client_ui_send_buffer, UI_Data_Del_ALL, 0);
			CLIENT_UI_SEND_BUFFER();
			vTaskDelay(UI_SEND_DELAY_TIME_MS);
			goto WAIT_UI_RUN;
		}
		CLIENT_UI_SEND_BUFFER();
	}

	//vTaskDelete(NULL);
}

/**
 * @brief 顶级准星绘制
 * 
 */
static void Front_Sight_Draw(void)
{
	Line_Draw(&static_graph, "CS1", UI_Graph_ADD, 0, UI_Color_Cyan, 3, 961, 550, 961, 600); //上
	UI_ReFresh(client_ui_send_buffer, 1, static_graph);
	CLIENT_UI_SEND_BUFFER();
	Line_Draw(&static_graph, "CS2", UI_Graph_ADD, 0, UI_Color_Cyan, 3, 961, 530, 961, 480); //下
	UI_ReFresh(client_ui_send_buffer, 1, static_graph);
	CLIENT_UI_SEND_BUFFER();
	Line_Draw(&static_graph, "CS3", UI_Graph_ADD, 0, UI_Color_Cyan, 3, 950, 541, 900, 541); //左
	UI_ReFresh(client_ui_send_buffer, 1, static_graph);
	CLIENT_UI_SEND_BUFFER();
	Line_Draw(&static_graph, "CS4", UI_Graph_ADD, 0, UI_Color_Cyan, 3, 970, 541, 1020, 541); //右
	UI_ReFresh(client_ui_send_buffer, 1, static_graph);
	CLIENT_UI_SEND_BUFFER();
}

void ChangeClientUiDrawStatus(void) {
	if (draw_status) {
		draw_status = 0;
	} else {
		draw_status = 1;
	}
}
