#include "xbee_protocol.h"
#include "xbee_atcmd.h"
#include "xbee_bsp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <limits.h>
#include "serial.h"
#include "server_duty.h"

/******************************************************
**uart接收校验
******************************************************/
int16 UartRevDataProcess(uint8* UartRevBuf)
{
	int16 UartRevLen;
	uint16 DataLen,i;
	uint8 checksum,temp;

	UartRevLen =	ReadComPort(UartRevBuf,1);
	if(UartRevLen == 0)
		return 0;
	if(UartRevBuf[0] != 0x7E)
		return 0;
	UartRevLen =	ReadComPort(UartRevBuf+1,2);
	if(UartRevLen < 2)
		return 0;
	temp = UartRevBuf[1];
	i = (uint16)temp << 8;
	temp = UartRevBuf[2];
	DataLen = i + (uint16)UartRevBuf[2];
	UartRevLen =	ReadComPort(UartRevBuf+3,DataLen+1);
	checksum = XBeeApiChecksum(UartRevBuf+3,DataLen); //校验数据
	if(checksum != UartRevBuf[DataLen+3])
		return 0;
	else
		return DataLen+4;
}
/*******************************************************
**brief 处理接收到的CFG指令
*******************************************************/
void XBeeProcessCFG(uint8 *rbuf)
{	
	int16 temp;
	switch(*(rbuf+18))
	{
		case net_request:
			temp = get_local_addr(rbuf+12,rbuf+4); //调用API 查询是否属于网络
			printf("查询返回值是%d \n",temp);
			if(get_local_addr(rbuf+12,rbuf+4) == 1)	//属于该网络,允许加入网络，并设定允许加入网络时间
			//if(1)			
			{
				XBeeJionEnable((rbuf+4),(rbuf+12)); //
				//set_node_online(rbuf+4);
				XBeeSendTimeout(0xc8); //限时加入网络2m
				XBeeSendSenserInit((rbuf+4),(rbuf+12)); 
			}	
			else
				XBeeJionDisable((rbuf+19),(rbuf+27));
			break;
		default:
			break;
	}
	return;
}
/*******************************************************
**brief 处理接收到的CTL指令
*******************************************************/
void XBeeProcessCTL(uint8 *rbuf)
{
	if(*(rbuf+18) == lock_response)
	{
		switch(*(rbuf+19))
		{
			case ParkUnlockSuccess:
				
				break;
			case ParkLockSuccess:	
				
				break;
			case ParkUnlockFailed:
				
				break;
			case ParkLockFailed:

				break;
			default:
				break;
		}
	}
}
/*******************************************************
**brief 处理接收到的SEN指令
*******************************************************/
void XBeeProcessSEN(uint8 *rbuf)
{
	switch(*(rbuf+18))
	{
		case lock_event:
			if(*(rbuf+19) == ParkingUsed)
				event_report( char_to_int(rbuf+12),en_vehicle_comming);
			else if(*(rbuf+19) == ParkingUnUsed)
				event_report( char_to_int(rbuf+12),en_vehicle_leave);
			else if(*(rbuf+19) == ParkLockSuccess)
				event_report( char_to_int(rbuf+12),en_lock_success);
			else if(*(rbuf+19) == ParkLockFailed)
				event_report( char_to_int(rbuf+12),en_lock_failed);
			else if(*(rbuf+19) == ParkUnlockSuccess)
				event_report( char_to_int(rbuf+12),en_unlock_success);
			else if(*(rbuf+19) == ParkUnlockFailed)
				event_report( char_to_int(rbuf+12),en_unlock_failed);
			break;
		case bat_event:
			break;
		default:
			break;
	}
	return;
}
/*************************************************
**向router发送限时加入网络命令
*************************************************/
int16 XBeeSendTimeout(uint8 time)
{
	uint8 data[5];
	
	data[0]  =  'C';
	data[1]  =  'F';
	data[2]  =  'G';
	data[3]  = 0X00;
	data[4]  = time;
	return XBeeBoardcastTrans(data,5,RES);
}
/**************************************************
**brief 允许入网
**************************************************/
int16 XBeeJionEnable(uint8 *ieeeadr,uint8 *netadr)
{
	uint8 data[5];
	data[0]  =  'C';
	data[1]  =  'F';
	data[2]  =  'G';
	data[3]  =  0x02;
	data[4]  =  0x01;
	return XBeeTransReq(ieeeadr,netadr,Default,data,5,RES);
}
/**************************************************
**brief 禁止入网
**************************************************/
int16 XBeeJionDisable(uint8 *ieeeadr,uint8 *netadr)
{
	uint8 data[5];
	data[0]  =  'C';
	data[1]  =  'F';
	data[2]  =  'G';
	data[3]  =  0x02;
	data[4]  =  0x00;
	return XBeeTransReq(ieeeadr,netadr,Default,data,5,RES);
}
/*******************************************************
**brief 发送恢复出厂设置指令
*******************************************************/
int16 XBeeSendFactorySettingCmd(uint8 *ieeeadr,uint8 *netadr)
{
	uint8 data[4];
	data[0]  =  'C';
	data[1]  =  'F';
	data[2]  =  'G';
	data[3]  =  0x03;
	return XBeeTransReq(ieeeadr,netadr,Default,data,4,RES);
}
/*******************************************************
**brief 终端控制请求   调用后台获得锁的地址
**param	ieeeadr	目标的8字节地址
		netadr	目标的网络地址
		lockstate 0 解锁
				  1 锁定
*******************************************************/
int16 XBeePutCtlCmd(uint8 *ieeeadr,uint16 netadr,uint8 lockstate)
{
	uint8 data[5];
	data[0]  =  'C';	
	data[1]  =  'T';
	data[2]  =  'L';
	data[3]  =  0x00;
	data[4]  =  lockstate;
	uint8 netadr_s[2];
	netadr_s[0] = (uint8)(netadr>>8);
	netadr_s[1] = (uint8)netadr;
	return XBeeTransReq(ieeeadr,netadr_s,Default,data,5,RES);
}
/**************************************************************
**brief 标定senser初始值
**************************************************************/
int16 XBeeSendSenserInit(uint8 *ieeeadr,uint8 *net_adr)
{
	uint8 data[4];
	
	data[0]    =    'S';
	data[1]    =    'E';
	data[2]    =    'N';
	data[3]    =      0;
	return XBeeTransReq(ieeeadr,net_adr,Default,data,4,NO_RES);
}
/**********************************************************
**brief 锁控制命令
**param netadr 目标锁的16位网络抵制
		lockstate 0x00  解锁
				  0x01  上锁
××reval 发送数据的长度
**********************************************************/
int16 putCtlCmd(uint16 netadr,uint8 lockstate)
{
	uint8 net_adr[2],adr[8],data[5],i;		
	for(i=0;i<8;i++)
		adr[i] = 0;
	net_adr[1] = (uint8)netadr; //大端序，小端序
	net_adr[0] = (uint8)(netadr>>8);
	data[0]  =  'C';	
	data[0]  =  'T';
	data[1]  =  'L';
	data[3]  =  0x00;
	data[4]  =  lockstate;
	return XBeeUnicastTrans(adr,net_adr,Default,data,5,RES);
}

/***************************************************************
**brief 两个字节合并一个unt16
***************************************************************/
uint16 char_to_int(uint8 *data)
{
	uint16 reval=0;
	reval |= (uint16)*(data+1);
	reval |= ((uint16)*data << 8);
	return reval;
}















