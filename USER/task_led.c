#include "task_led.h"
#include "led.h"
#include "delay.h"


TaskHandle_t xHandleTaskLED = NULL;

void vTaskLED(void *pvParameters)
{
	while(1)
	{
		IWDG_Feed();
		
		delay_ms(3000);
		RUN_LED = 0;
		delay_ms(30);
		RUN_LED = 1;
	}
}






































