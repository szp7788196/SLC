#include "task_main.h"
#include "delay.h"


TaskHandle_t xHandleTaskMAIN = NULL;

void vTaskMAIN(void *pvParameters)
{
	while(1)
	{
		delay_ms(100);
	}
}






































