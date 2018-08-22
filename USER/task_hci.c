#include "task_hci.h"
#include "delay.h"
#include "usart.h"


TaskHandle_t xHandleTaskHCI = NULL;

void vTaskHCI(void *pvParameters)
{
	while(1)
	{
		if(Usart1RecvEnd == 0xAA)
		{
			Usart1RecvEnd = 0;
			UsartSendString(USART1,(u8 *)"123456", 6);
		}
		
		delay_ms(50);
	}
}






































