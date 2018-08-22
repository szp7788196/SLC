#include "task_sensor.h"
#include "delay.h"
#include "sht2x.h"
#include "bh1750.h"


TaskHandle_t xHandleTaskSENSOR = NULL;

void vTaskSENSOR(void *pvParameters)
{
	SHT2x_Init();
	Bh1750_Init();
	
	while(1)
	{
		Temperature = Sht2xReadTemperature();
		Humidity = Sht2xReadHumidity();
		
		Illumination = Bh1750ReadIllumination();
		
		delay_ms(500);
	}
}






































