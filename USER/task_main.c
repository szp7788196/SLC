#include "task_main.h"
#include "common.h"
#include "delay.h"
#include "inventr.h"


TaskHandle_t xHandleTaskMAIN = NULL;

u8 MirrorLightLevelPercent = 0;

struct tm tm_time;

time_t timesss = 0;



void vTaskMAIN(void *pvParameters)
{
	InventrSetLightLevel(INIT_LIGHT_LEVEL);					//ÉÏµçÄ¬ÈÏÃðµÆ
	
	tm_time.tm_sec = 10;
	tm_time.tm_min = 10;
	tm_time.tm_hour = 10;
	tm_time.tm_mday = 10;
	tm_time.tm_mon = 9;
	tm_time.tm_year = 118;
	
	timesss = mktime(&tm_time);
	
	while(1)
	{
		if(MirrorLightLevelPercent != LightLevelPercent)
		{
			MirrorLightLevelPercent = LightLevelPercent;
			
			InventrSetLightLevel(LightLevelPercent);
		}
		
		delay_ms(100);
	}
}






































