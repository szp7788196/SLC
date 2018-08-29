#include "task_net.h"
#include "common.h"
#include "delay.h"
#include "tcp.h"
#include "net_protocol.h"


TaskHandle_t xHandleTaskNET = NULL;

CONNECT_STATE_E ConnectState = UNKNOW_ERROR;	//bg96的连接状态
u8 SignalIntensity = 99;						//bg96的信号强度

SensorMsg_S *p_tSensorMsgNet = NULL;			//用于装在传感器数据的结构体变量




void vTaskNET(void *pvParameters)
{
	time_t times_sec = 0;
	
	BG96_InitStep1(&bg96);
	BG96_InitStep2(&bg96);
	
	Tcp_Init(&bg96,&tcp);
	
	p_tSensorMsgNet = (SensorMsg_S *)mymalloc(sizeof(SensorMsg_S));
	
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
				OnServerHandle();

				
			break;

			default:

			break;
		}
		
		delay_ms(100);
	}
}

//尝试连接服务器
u8 TryToConnectToServer(void)
{
	u8 ret = 0;
	
	ret = tcp->connect(&tcp,(char *)ServerIP,(char *)ServerPort);
	
	return ret;
}

//在线处理进程
void OnServerHandle(void)
{
	u8 len = 0;
	u8 out_buf[512];
	
	SendSensorData_HeartBeatPacket();		//向服务器定时发送传感器数据和心跳包
	
	len = NetDataFrameHandle(&tcp,out_buf,HoldReg,ConnectState);
	
	if(len > 0)
	{
		len = tcp->send(&tcp, out_buf, len);
	}
}

//向服务器定时发送传感器数据和心跳包
void SendSensorData_HeartBeatPacket(void)
{
	static time_t times_sec = 0;
	BaseType_t xResult;
	u8 send_len = 0;
	u8 sensor_data_len = 0;
	
	u8 sensor_buf[128];
	u8 send_buf[512];
	
	xResult = xQueueReceive(xQueue_sensor,
							(void *)p_tSensorMsgNet,
							(TickType_t)pdMS_TO_TICKS(50));
	if(xResult == pdPASS)
	{
		memset(send_buf,0,512);
		memset(sensor_buf,0,128);
		
		sensor_data_len = UnPackSensorData(p_tSensorMsgNet,sensor_buf);
		
		send_len = PackNetData(0xE0,sensor_buf,sensor_data_len,send_buf);
	}
	else if(GetSysTick1s() - times_sec >= 20)
	{
		times_sec = GetSysTick1s();
		
		memset(send_buf,0,512);
		memset(sensor_buf,0,128);
		
		send_len = PackNetData(0xE1,sensor_buf,0,send_buf);
	}
	
	send_len = tcp->send(&tcp, send_buf, send_len);
	
	send_len = send_len;
}






































