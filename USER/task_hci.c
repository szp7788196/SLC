#include "task_hci.h"
#include "delay.h"
#include "usart.h"
#include "at_protocol.h"


TaskHandle_t xHandleTaskHCI = NULL;

void vTaskHCI(void *pvParameters)
{
	u16 send_len = 0;
	
	AT_CommandInit();
	
	UsartSendString(USART1,"READY\r\n", 7);
	
	while(1)
	{
		if(Usart1RecvEnd == 0xAA)
		{
			Usart1RecvEnd = 0;
			
			send_len = AT_CommandDataAnalysis(Usart1RxBuf,Usart1FrameLen,Usart1TxBuf,HoldReg);
			
			memset(Usart1RxBuf,0,Usart1FrameLen);
		}
		
		if(send_len != 0)
		{
			UsartSendString(USART1,Usart1TxBuf, send_len);
			
			memset(Usart1TxBuf,0,send_len);
			
			send_len = 0;
		}
		
		delay_ms(100);
	}
}






































