#include "common.h"
#include "24cxx.h"
#include "sht2x.h"
#include "bh1750.h"
#include "led.h"

//u16 i = 0;
//u8 eepbuf[256];
u16 cnt = 0;
u8 led_s = 0;
RCC_ClocksTypeDef RCC_Clocks;
int main(void)
{
//	IWDG_Init(IWDG_Prescaler_128,3750);	//128分频 312.5HZ 3750为12秒
	RCC_GetClocksFreq(&RCC_Clocks);
	__set_PRIMASK(1);	//关闭所有中断
	delay_init(72);
	NVIC_Configuration();
	
	AT24CXX_Init();
	SHT2x_Init();
	Bh1750_Init();
	LED_Init();
	
//	for(i = 0; i < 256; i ++)
//	{
//		AT24CXX_WriteOneByte(i,i);
//	}
//	for(i = 0; i < 256; i ++)
//	{
//		eepbuf[i] = AT24CXX_ReadOneByte(i);
//	}
	
	mem_init();

	IWDG_Feed();	//喂开门狗

    /* 启动调度，开始执行任务 */
//    vTaskStartScheduler();

	while(1)
	{
		Temperature = Sht2xReadTemperature();
		Humidity = Sht2xReadHumidity();
		
		Illumination = Bh1750ReadIllumination();
		
		
		if(cnt % 20 == 0)
		{
			RUN_LED = 0;
		}
		else
		{
			RUN_LED = 1;
		}
		
		delay_ms(100);
		
		cnt = (cnt + 1) & 0xFFFF;
	}
}

























