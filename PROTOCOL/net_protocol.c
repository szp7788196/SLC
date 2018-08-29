#include "net_protocol.h"
#include "rtc.h"
#include "usart.h"
#include "24cxx.h"
#include "common.h"

//读取/处理网络数据
u16 NetDataFrameHandle(pTcp *tcp,u8 *outbuf,u8 *hold_reg,CONNECT_STATE_E connect_state)
{
	u16 ret = 0;
	u16 len = 0;
	u8 buf[1024];

	memset(buf,0,1024);

	len = (*tcp)->read(tcp,buf);
	if(len != 0)
	{
		if(connect_state == ON_SERVER)
		{
			ret = NetDataAnalysis(buf,len,outbuf,hold_reg);
		}
		memset(buf,0,len);
	}
	return ret;
}

//网络数据帧协议解析
u16 NetDataAnalysis(u8 *buf,u16 len,u8 *outbuf,u8 *hold_reg)
{
	u16 ret = 0;
	u16 pos = 0;
	u16 data_len = 0;

	u8 cmd_code = 0;
	u8 read_check_sum = 0;
	u8 cal_check_sum = 0;
	
	u8 buf_tail[6] = {0xFE,0xFD,0xFC,0xFB,0xFA,0xF9};

	pos = MyStrstr(buf,buf_tail,len,6);

	if(pos != 0xFFFF)
	{
		if(*(buf + 0) == 0x68 && \
			*(buf + 7) == 0x68 && \
			*(buf + pos - 1) == 0x16)								//判断包头和包尾
		{
			if(MyStrstr(buf + 1,DeviceID,pos,6) != 0xFFFF)			//判断设备ID
			{
				cmd_code = *(buf + 8);								//获取功能码
				data_len = *(buf + 9);								//获取有效数据的长度
				read_check_sum = *(buf + pos - 2);					//获取校验和
				cal_check_sum = CalCheckSum(buf, pos - 2);			//计算校验和
				
				if(read_check_sum == cal_check_sum)
				{
					switch(cmd_code)
					{
						case 0xE0:									//发送固定信息，上行，在别处处理
							
						break;

						case 0xE1:									//发送心跳，上行，在别处处理
							
						break;

						case 0xE2:									//开关灯/调光，下行
							ret = ControlLightLevel(cmd_code,buf + 10,data_len,outbuf);
						break;

						case 0xE3:									//远程升级OTA，下行
							
						break;

						case 0xE4:									//重启/复位，下行
							ret = ControlDeviceReset(cmd_code,buf + 10,data_len,outbuf);
						break;

						case 0xE5:									//设置定时发送间隔,下行
							
						break;

						case 0xE6:									//控制柜断电/通电，下行
							
						break;

						case 0xE7:									//设置亮灭灯定时策略，下行
							
						break;

						case 0xE8:									//读取/发送设备配置信息，下行
							
						break;

						case 0x80:									//应答，下行,上行在别处处理
							UnPackAckPacket(cmd_code,buf + 10,data_len);
						break;

						default:									//此处要给云端应答一个功能码错误信息
							
						break;
					}
				}
			}
		}
		else	//此处可以给云端应答一个校验错误信息
		{
			
		}
	}
	else		//此处可以给云端应答一个校验错误信息
	{
		
	}
	
	return ret;
}


//解析ACK包
u8 UnPackAckPacket(u8 cmd_code,u8 *buf,u8 len)
{
	u8 ret = 0;
	
	if(len == 2)
	{
		if(*(buf + 0) == cmd_code)
		{
			if(*(buf + 1) == 0)
			{
				ret = 1;
			}
		}
	}
	
	return ret;
}

//ACK打包
u16 PackAckPacket(u8 cmd_code,u8 *data,u8 *outbuf)
{
	u16 len = 0;
	
	len = PackNetData(0x80,data,2,outbuf);
	
	return len;
}

//控制灯的亮度
u16 ControlLightLevel(u8 cmd_code,u8 *buf,u8 len,u8 *outbuf)
{
	u8 out_len = 0;
	u8 level = 0;
	u8 data_buf[2] = {0,0};
	data_buf[0] = cmd_code;
	
	if(len == 1)
	{
		level = *(buf + 0);
		
		if(level <= 100)
		{
			LightLevelPercent = 2 * level;
		}
		else
		{
			data_buf[1] = 1;
		}
	}
	else
	{
		data_buf[1] = 2;
	}
	
	out_len = PackAckPacket(cmd_code,data_buf,outbuf);
	
	return out_len;
}

//远程重启
u16 ControlDeviceReset(u8 cmd_code,u8 *buf,u8 len,u8 *outbuf)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};
	data_buf[0] = cmd_code;
	
	if(len == 0)
	{
		NeedToReset = 1;
	}
	else
	{
		data_buf[1] = 2;
	}
	
	out_len = PackAckPacket(cmd_code,data_buf,outbuf);
	
	return out_len;
}






































