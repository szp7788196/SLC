#include "task_main.h"
#include "delay.h"
#include "inventr.h"


TaskHandle_t xHandleTaskMAIN = NULL;

u8 LightLevel = 0;

void vTaskMAIN(void *pvParameters)
{
	while(1)
	{
		delay_ms(500);
		
		InventrSetMaxPowerCurrent(100);
		
		LightLevel += 20;
		
		if(LightLevel > 200)
		{
			LightLevel = 20;
		}
		
		InventrSetLightLevel(20);
		
		delay_ms(500);
		InventrOutPutCurrent = InventrGetOutPutCurrent();
		delay_ms(500);
		InventrOutPutVoltage = InventrGetOutPutVoltage();
		delay_ms(500);
		InventrGetDeviceInfo();
	}
}






































