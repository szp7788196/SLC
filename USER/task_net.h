#ifndef __TASK_NET_H
#define __TASK_NET_H

#include "sys.h"
#include "rtos_task.h"

extern TaskHandle_t xHandleTaskNET;

void vTaskNET(void *pvParameters);




u8 TryToConnectToServer(void);
void OnServerHandle(void);

































#endif
