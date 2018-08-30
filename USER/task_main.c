#include "task_main.h"
#include "common.h"
#include "delay.h"
#include "inventr.h"


TaskHandle_t xHandleTaskMAIN = NULL;

u8 MirrorLightLevelPercent = 0;


void vTaskMAIN(void *pvParameters)
{
	InventrSetLightLevel(INIT_LIGHT_LEVEL);					//�ϵ�Ĭ�����
	
	while(1)
	{
		if(MirrorLightLevelPercent != LightLevelPercent)
		{
			MirrorLightLevelPercent = LightLevelPercent;
			
			InventrSetLightLevel(LightLevelPercent);
		}
		
		
		if(NeedToReset == 1)			//���յ�����������
		{
			NeedToReset = 0;
			delay_ms(1000);
			
			__disable_fault_irq();		//����ָ��
			NVIC_SystemReset();
		}
		delay_ms(100);
	}
}






































