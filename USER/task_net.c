#include "task_net.h"
#include "delay.h"
#include "bg96.h"
#include "tcp.h"


TaskHandle_t xHandleTaskNET = NULL;


void vTaskNET(void *pvParameters)
{
	BG96_InitStep1(&bg96);
	BG96_InitStep2(&bg96);
	
	Tcp_Init(&bg96,&tcp);
	
	while(1)
	{
		delay_ms(100);
	}
}






































