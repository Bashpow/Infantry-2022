#ifndef __SHELL_TASK_H
#define __SHELL_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

extern TaskHandle_t ShellTask_Handler;

void Shell_Task(void *pvParameters);

void Shell_Get_Data(void);

#endif
