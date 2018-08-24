#include "common.h"
#include "24cxx.h"


u8 HoldReg[HOLD_REG_LEN];				//保持寄存器

SemaphoreHandle_t  xMutex_IIC1 = NULL;	//IIC总线1的互斥量

/***************************固件升级相关*****************************/
u8 NeedUpDateFirmWare = 0;			//有新固件需要加载
u8 HaveNewFirmWare = 0;				//0xAA有新固件 others无新固件
u8 NewFirmWareAdd = 0;				//0xAA新固件地址0x0800C000 0x55新固件地址0x08026000
u16 NewFirmWareBagNum = 0;			//固件包的数量（一个固件包含多个小包）
u16 NewFirmWareVer = 1;				//固件包的版本
u8 LastBagByteNum = 0;				//最后一包的字节数

/***************************系统心跳相关*****************************/
u32 SysTick1ms = 0;					//1ms滴答时钟
u32 SysTick10ms = 0;				//10ms滴答时钟
u32 SysTick100ms = 0;				//10ms滴答时钟
time_t SysTick1s = 0;				//1s滴答时钟

/***************************版本相关*********************************/
u8 *BootLoaderVersion = NULL;		//BootLoader版本号
u8 *SoftWareVersion = NULL;			//应用程序版本号
u8 *HardWareVersion = NULL;			//硬件版本号

/***************************设备相关*********************************/
u8 *DeviceName = NULL;				//设备名称
u8 *DeviceID = NULL;				//设备ID
u8 *UUID = NULL;					//设备UUID

/***************************网络相关*********************************/
u8 Operators = 0;					//运营商编号
u8 *APN = NULL;						//私有APN，不同客户APN不同
u8 *ServerDomain = NULL;			//服务器域名
u8 *ServerIP = NULL;				//服务器IP地址
u8 *ServerPort = NULL;				//服务器端口号

/***************************运行参数相关*****************************/
u16 UpLoadINCL = 10;				//数据上传时间间隔0~65535秒
u8 PowerINTFC = 2;					//电源控制接口编号 0:0~10V  1:PWM  2:UART
u8 TimeZone = 8;					//时区偏移量


//在str1中查找str2，失败返回0xFF,成功返回str2首个元素在str1中的位置
u16 MyStrstr(u8 *str1, u8 *str2, u16 str1_len, u16 str2_len)
{
	u16 len = str1_len;
	if(str1_len == 0 || str2_len == 0)	
	{
		return 0xFFFF;
	}
	else
	{
		while(str1_len >= str2_len)
		{
			str1_len --;
			if (!memcmp(str1, str2, str2_len))
			{
				return len - str1_len - 1;
			}
			str1 ++;
		}
		return 0xFFFF;
	}
}

//获得整数的位数
u8 GetDatBit(u32 dat)
{
	u8 j = 1;
	u32 i;
	i = dat;
	while(i >= 10)
	{
		j ++;
		i /= 10;
	}
	return j;
}

//用个位数换算出一个整数 1 10 100 1000......
u32 GetADV(u8 len)
{
	u32 count = 1;
	if(len == 1)
	{
		return 1;
	}
	else
	{
		len --;
		while(len --)
		{
			count *= 10;
		}
	}
	return count;
}

//整数转换为字符串
void IntToString(u8 *DString,u32 Dint,u8 zero_num)
{
	u16 i = 0;  
	u8 j = GetDatBit(Dint);
	for(i = 0; i < GetDatBit(Dint) + zero_num; i ++)
	{ 
		DString[i] = Dint / GetADV(j) % 10 + 0x30;
		j --;
	}
}

u32 StringToInt(u8 *String)
{
	u8 len;
	u8 i;
	u32 count=0;
	u32 dev;
	
	len = strlen((char *)String);
	dev = 1;
	for(i = 0; i < len; i ++)//len-1
	{
		if(String[i] != '.')
		{
			count += ((String[i] - 0x30) * GetADV(len) / dev);
			dev *= 10;
		}
		else
		{
			len --;
			count /= 10;
		}
	}
	if(String[i]!=0x00)
	{
		count += (String[i] - 0x30);
	}
	return count;
}
 
//32位CRC校验
u32 CRC32( const u8 *buf, u32 size)
{
     uint32_t i, crc;
     crc = 0xFFFFFFFF;
     for (i = 0; i < size; i++)
      crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
     return crc^0xFFFFFFFF;
}

/***************************************************** 
函数：u16 CRC16(u8 *puchMsgg,u8 usDataLen)
功能：CRC校验用函数
参数：puchMsgg是要进行CRC校验的消息，usDataLen是消息中字节数														    
返回：计算出来的CRC校验码。                                                                                      
*****************************************************/
u16 CRC16(u8 *puchMsgg,u8 usDataLen)
{
    u8 uchCRCHi = 0xFF ; 											//高CRC字节初始化
    u8 uchCRCLo = 0xFF ; 											//低CRC 字节初始化
    u8 uIndex ; 													//CRC循环中的索引
    while (usDataLen--) 											//传输消息缓冲区
    {
		uIndex = uchCRCHi ^ *puchMsgg++; 							//计算CRC 
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
		uchCRCLo = auchCRCLo[uIndex];
    }
    return ((uchCRCHi << 8) | uchCRCLo);
}

//产生一个系统1毫秒滴答时钟.
void SysTick1msAdder(void)
{	
	SysTick1ms = (SysTick1ms + 1) & 0xFFFFFFFF;
}

//获取系统1毫秒滴答时钟
u32 GetSysTick1ms(void)
{
	return SysTick1ms;
}

//产生一个系统10毫秒滴答时钟.
void SysTick10msAdder(void)
{
	SysTick10ms = (SysTick10ms + 1) & 0xFFFFFFFF;
}

//获取系统10毫秒滴答时钟
u32 GetSysTick10ms(void)
{
	return SysTick10ms;
}

//产生一个系统100毫秒滴答时钟.
void SysTick100msAdder(void)
{	
	SysTick100ms = (SysTick100ms + 1) & 0xFFFFFFFF;
}

//获取系统100毫秒滴答时钟
u32 GetSysTick100ms(void)
{
	return SysTick1ms;
}

void SetSysTick1s(time_t sec)
{
	SysTick1s = sec;
}

//获取系统1秒滴答时钟
time_t GetSysTick1s(void)
{
	return SysTick1s;
}

//从EEPROM中读取数据(带CRC16校验码)
u8 ReadDataFromEepromToHoldBuf(u8 *inbuf,u16 s_add, u16 len)
{
	u8 i = 0;
	u16 ReadCrcCode;
	u16 CalCrcCode = 0;

	for(i = s_add; i < len; i ++)
	{
		*(inbuf + i) = AT24CXX_ReadOneByte(i);
	}

	ReadCrcCode=(u16)(*(inbuf + len - 2));
	ReadCrcCode=ReadCrcCode<<8;
	ReadCrcCode=ReadCrcCode|(u16)(u16)(*(inbuf + len - 1));

	CalCrcCode = CRC16(inbuf + s_add,len - 2);

	if(ReadCrcCode == CalCrcCode)
	{
		return 1;
	}
	
	return 0;
}

//将数字或者缓冲区当中的数据转换成字符串，并赋值给相应的指针
//type 0:转换数字id 1:转换缓冲区数据，add为缓冲区起始地址 2将字符串长度传到参数size中
u8 GetMemoryForString(u8 **str, u8 type, u32 id, u16 add, u16 size, u8 *hold_reg)
{
	u8 ret = 0;
	u8 len = 0;
	u8 new_len = 0;

	if(*str == NULL)
	{
		if(type == 0)
		{
			len = GetDatBit(id);
		}
		else if(type == 1)
		{
			len = *(hold_reg + add);
		}
		else if(type == 2)
		{
			len = size;
			
			add -= 1;
		}

		*str = (u8 *)mymalloc(sizeof(u8) * len + 1);
	}
	if(*str != NULL)
	{
		len = strlen((char *)*str);
		if(type == 0)
		{
			new_len = GetDatBit(id);
		}
		else if(type == 1)
		{
			new_len = *(hold_reg + add);
		}
		else if(type == 2)
		{
			len = size;
			
			add -= 1;
		}

		if(len == new_len)
		{
			memset(*str,0,new_len + 1);

			if(type == 0)
			{
				IntToString(*str,id,0);
			}
			else if(type == 1 || type == 2)
			{
				memcpy(*str,(hold_reg + add + 1),new_len);
			}
			ret = 1;
		}
		else
		{
			myfree(*str);
			*str = (u8 *)mymalloc(sizeof(u8) * new_len + 1);
			if(*str != NULL)
			{
				memset(*str,0,new_len + 1);

				if(type == 0)
				{
					IntToString(*str,id,0);
				}
				else if(type == 1 || type == 2)
				{
					memcpy(*str,(hold_reg + add + 1),new_len);
				}
				len = new_len;
				new_len = 0;
				ret = 1;
			}
		}
	}

	return ret;
}

//获取设备名称
u8 GetDeviceName(void)
{
	u8 ret = 0;

	ret = GetMemoryForString(&DeviceName, 1, 0, DEVICE_NAME_ADD, 0,HoldReg);

	return ret;
}

//获取设备ID
u8 GetDeviceID(void)
{
	u8 ret = 0;

	ret = GetMemoryForString(&DeviceID, 2, 0, DEVICE_ID_ADD, DEVICE_ID_LEN - 2, HoldReg);

	return ret;
}

//获取设备UUID
u8 GetDeviceUUID(void)
{
	u8 ret = 0;

	ret = GetMemoryForString(&UUID, 2, 0, UU_ID_ADD, UU_ID_LEN - 2, HoldReg);

	return ret;
}

//获取APN
u8 GetAPN(void)
{
	u8 ret = 0;

	ret = GetMemoryForString(&APN, 2, 0, APN_ADD, APN_LEN - 2, HoldReg);

	return ret;
}

//获取服务器域名
u8 GetServerDomain(void)
{
	u8 ret = 0;

	ret = GetMemoryForString(&ServerDomain, 2, 0, SERVER_DOMAIN_ADD, SERVER_DOMAIN_LEN - 2, HoldReg);

	return ret;
}

//获取服务器IP
u8 GetServerIP(void)
{
	u8 ret = 0;

	ret = GetMemoryForString(&ServerIP, 2, 0, SERVER_IP_ADD, SERVER_IP_LEN - 2, HoldReg);

	return ret;
}

//获取服务器Port
u8 GetServerPort(void)
{
	u8 ret = 0;

	ret = GetMemoryForString(&ServerPort, 2, 0, SERVER_PORT_ADD, SERVER_PORT_LEN - 2, HoldReg);

	return ret;
}

//读取BootLoader版本号
u8 ReadBootLoaderVersion(void)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(HoldReg,BOOT_SW_VER_ADD, BOOT_SW_VER_LEN);
	
	if(ret)
	{
		if(BootLoaderVersion == NULL)
		{
			BootLoaderVersion = (u8 *)mymalloc(sizeof(u8) * 6);
		}
		
		memset(BootLoaderVersion,0,6);
		
		sprintf((char *)BootLoaderVersion, "%02d.%02d", HoldReg[BOOT_SW_VER_ADD + 0],HoldReg[BOOT_SW_VER_ADD + 1]);
	}
	else
	{
		if(BootLoaderVersion == NULL)
		{
			BootLoaderVersion = (u8 *)mymalloc(sizeof(u8) * 6);
		}
		
		memset(BootLoaderVersion,0,6);
		
		sprintf((char *)BootLoaderVersion, "null");
	}
	
	return ret;
}

//读取应用程序版本号
u8 ReadSoftWareVersion(void)
{
	u8 ret = 1;
	
	if(SoftWareVersion == NULL)
	{
		SoftWareVersion = (u8 *)mymalloc(sizeof(u8) * 6);
	}
	
	memset(SoftWareVersion,0,6);
	
	sprintf((char *)SoftWareVersion, "%02d.%02d", SOFT_WARE_VRESION / 100,SOFT_WARE_VRESION % 100);
	
	return ret;
}

//读取硬件版本号
u8 ReadHardWareVersion(void)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(HoldReg,HW_VER_ADD, HW_VER_LEN);
	
	if(ret)
	{
		if(HardWareVersion == NULL)
		{
			HardWareVersion = (u8 *)mymalloc(sizeof(u8) * 6);
		}
		
		memset(HardWareVersion,0,6);
		
		sprintf((char *)HardWareVersion, "%02d.%02d", HoldReg[HW_VER_ADD + 0],HoldReg[HW_VER_ADD + 1]);
	}
	else
	{
		if(HardWareVersion == NULL)
		{
			HardWareVersion = (u8 *)mymalloc(sizeof(u8) * 6);
		}
		
		memset(HardWareVersion,0,6);
		
		sprintf((char *)HardWareVersion, "null");
	}
	
	return ret;
}

//读取设备名称
u8 ReadDeviceName(void)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(HoldReg,DEVICE_NAME_ADD, DEVICE_NAME_LEN);
	
	if(ret)
	{
		GetDeviceName();
	}
	else
	{
		if(DeviceName == NULL)
		{
			DeviceName = (u8 *)mymalloc(sizeof(u8) * 5);
		}
		
		memset(DeviceName,0,5);
		
		sprintf((char *)DeviceName, "null");
	}
	
	return ret;
}

//读取设备ID
u8 ReadDeviceID(void)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(HoldReg,DEVICE_ID_ADD, DEVICE_ID_LEN);
	
	if(ret)
	{
		GetDeviceID();
	}
	else
	{
		if(DeviceID == NULL)
		{
			DeviceID = (u8 *)mymalloc(sizeof(u8) * 6);
		}
		
		memset(DeviceID,0,6);
	}
	
	return ret;
}

//读取设备UUID
u8 ReadDeviceUUID(void)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(HoldReg,UU_ID_ADD, UU_ID_LEN);
	
	if(ret)
	{
		GetDeviceUUID();
	}
	else
	{
		if(UUID == NULL)
		{
			UUID = (u8 *)mymalloc(sizeof(u8) * 65);
		}
		
		memset(UUID,0,65);
		
		sprintf((char *)UUID, "0123456789012345678901234567890123456789012345678901234567890123");
	}
	
	return ret;
}

//读取运营商编号
u8 ReadOperators(void)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(HoldReg,OPERATORS_ADD, OPERATORS_LEN);
	
	if(ret)
	{
		if(HoldReg[OPERATORS_ADD] < 3)
		{
			Operators = HoldReg[OPERATORS_ADD];
		}
		else
		{
			Operators = 0;
		}
	}
	
	return ret;
}

//读取APN
u8 ReadAPN(void)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(HoldReg,APN_ADD, APN_LEN);
	
	if(ret)
	{
		GetAPN();
	}
	else
	{
		if(APN == NULL)
		{
			APN = (u8 *)mymalloc(sizeof(u8) * 5);
		}
		
		memset(APN,0,5);
		
		sprintf((char *)APN, "null");
	}
	
	return ret;
}

//读取服务器域名
u8 ReadServerDomain(void)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(HoldReg,SERVER_DOMAIN_ADD, SERVER_DOMAIN_LEN);
	
	if(ret)
	{
		GetServerDomain();
	}
	else
	{
		if(ServerDomain == NULL)
		{
			ServerDomain = (u8 *)mymalloc(sizeof(u8) * 19);
		}
		
		memset(ServerDomain,0,19);
		
		sprintf((char *)ServerDomain, "www.nnlighting.com");
	}
	
	return ret;
}

//读取服务器IP
u8 ReadServerIP(void)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(HoldReg,SERVER_IP_ADD, SERVER_IP_LEN);
	
	if(ret)
	{
		GetServerIP();
	}
	else
	{
		if(ServerIP == NULL)
		{
			ServerIP = (u8 *)mymalloc(sizeof(u8) * 16);
		}
		
		memset(ServerIP,0,16);
		
		sprintf((char *)ServerIP, "47.105.38.215");
	}
	
	return ret;
}

//读取服务器port
u8 ReadServerPort(void)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(HoldReg,SERVER_PORT_ADD, SERVER_PORT_LEN);
	
	if(ret)
	{
		GetServerPort();
	}
	else
	{
		if(ServerPort == NULL)
		{
			ServerPort = (u8 *)mymalloc(sizeof(u8) * 6);
		}
		
		memset(ServerPort,0,6);
		
		sprintf((char *)ServerPort, "56556");
	}
	
	return ret;
}

//读取数据上传间隔时间
u8 ReadUpLoadINVL(void)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(HoldReg,UPLOAD_INVL_ADD, UPLOAD_INVL_LEN);
	
	if(ret)
	{
		UpLoadINCL = (((u16)HoldReg[UPLOAD_INVL_ADD + 0]) << 8) + (u16)HoldReg[UPLOAD_INVL_ADD +1] & 0x00FF;
		
		if(UpLoadINCL > MAX_UPLOAD_INVL)
		{
			UpLoadINCL = 10;
		}
	}
	
	return ret;
}

//读取电源控制接口编号
u8 ReadPowerINTFCC(void)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(HoldReg,POWER_INTFC_ADD, POWER_INTFC_LEN);
	
	if(ret)
	{
		if(HoldReg[POWER_INTFC_ADD] < 3)
		{
			PowerINTFC = HoldReg[POWER_INTFC_ADD];
		}
		else
		{
			PowerINTFC = 2;
		}
	}
	
	return ret;
}

//读取时区偏移
u8 ReadTimeZone(void)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(HoldReg,TIME_ZONE_ADD, TIME_ZONE_LEN);
	
	if(ret)
	{
		if(HoldReg[TIME_ZONE_ADD] < 25)
		{
			TimeZone = HoldReg[TIME_ZONE_ADD];
		}
		else
		{
			TimeZone = 8;
		}
	}
	
	return ret;
}

void ReadParametersFromEEPROM(void)
{
	ReadBootLoaderVersion();
	ReadSoftWareVersion();
	ReadHardWareVersion();
	ReadDeviceName();
	ReadDeviceID();
	ReadDeviceUUID();
	ReadOperators();
	ReadAPN();
	ReadServerDomain();
	ReadServerIP();
	ReadServerPort();
	ReadUpLoadINVL();
	ReadPowerINTFCC();
	ReadTimeZone();
}













