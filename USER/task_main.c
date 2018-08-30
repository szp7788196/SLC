#include "task_main.h"
#include "common.h"
#include "delay.h"
#include "inventr.h"


TaskHandle_t xHandleTaskMAIN = NULL;

u8 MirrorLightLevelPercent = 0;


void vTaskMAIN(void *pvParameters)
{
	InventrSetLightLevel(INIT_LIGHT_LEVEL);					//上电默认灭灯
	
	while(1)
	{
		if(MirrorLightLevelPercent != LightLevelPercent)
		{
			MirrorLightLevelPercent = LightLevelPercent;
			
			InventrSetLightLevel(LightLevelPercent);
		}
		
		
		if(NeedToReset == 1)			//接收到重启的命令
		{
			NeedToReset = 0;
			delay_ms(1000);
			
			__disable_fault_irq();		//重启指令
			NVIC_SystemReset();
		}
		delay_ms(100);
	}
}






































