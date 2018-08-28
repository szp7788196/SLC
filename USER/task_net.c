#include "task_net.h"
#include "common.h"
#include "delay.h"
#include "bg96.h"
#include "tcp.h"


TaskHandle_t xHandleTaskNET = NULL;

CONNECT_STATE_E ConnectState = UNKNOW_ERROR;	//bg96的连接状态
u8 SignalIntensity = 99;						//bg96的信号强度


u16 send_ok = 0;
u16 rx_len = 0;
u16 RxLen = 0;
u8 rx_buf[255];
void OnServerHandle(void)
{
	u8 ret = 0;
	
	ret = tcp->send(&tcp, (u8 *)"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", 100);
	
	if(ret == 10)
	{
		send_ok ++;
	}
}

void vTaskNET(void *pvParameters)
{
	time_t times_sec = 0;
	time_t times_sec1 = 0;
	
	BG96_InitStep1(&bg96);
	BG96_InitStep2(&bg96);
	
	Tcp_Init(&bg96,&tcp);
	
	while(1)
	{
		if(ConnectState == ON_SERVER)
		{
			if(GetSysTick1s() - times_sec >= 5)
			{
				times_sec = GetSysTick1s();
				ConnectState = bg96->get_connect_state(&bg96);
				SignalIntensity = bg96->get_AT_CSQ(&bg96);
			}
		}
		else
		{
			ConnectState = bg96->get_connect_state(&bg96);
			SignalIntensity = bg96->get_AT_CSQ(&bg96);
		}
		
		switch((u8)ConnectState)
		{
			case (u8)UNKNOW_ERROR:					//未知错误，重启模块
				BG96_InitStep2(&bg96);
			break;

			case (u8)GET_READY:						//模块已经就绪
				TryToConnectToServer();
			break;

			case (u8)NEED_CLOSE:					//需要关闭移动场景
				tcp->close(&tcp);
			break;

			case (u8)NEED_WAIT:						//延时等待
				delay_ms(2000);
			break;

			case (u8)ON_SERVER:						//已经连接到服务器
				if(GetSysTick1s() - times_sec1 >= 5)
				{
					times_sec1 = GetSysTick1s();
					OnServerHandle();
				}
				
				rx_len = tcp->read(&tcp,rx_buf);
	
				if(rx_len > 0)
				{
					RxLen = rx_len;
				}
				
			break;

			default:

			break;
		}
		
		delay_ms(100);
	}
}


u8 TryToConnectToServer(void)
{
	u8 ret = 0;
	
	ret = tcp->connect(&tcp,(char *)ServerIP,(char *)ServerPort);
	
	return ret;
}



































