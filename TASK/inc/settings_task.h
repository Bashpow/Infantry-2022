#ifndef __SETTINGS_TASK_
#define __SETTINGS_TASK_

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t SettingsTask_Handler;
void Settings_Task(void *pvParameters);

#endif // __SETTINGS_TASK_
