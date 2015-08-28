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
			if(temp == 0)	//属于该网络,允许加入网络，并设定允许加入网络时间		
			{
				XBeeJionEnable((rbuf+4),(rbuf+12)); //
				printf("\033[33m\033[1m已发送允许入网指令 \033[0m \n");	
				set_node_online(rbuf+4);
				printf("\033[33m\033[1m已将锁加入网络 \033[0m \n");	
				//XBeeSendTimeout(0xc8); //限时加入网络2m
				//XBeeSendSenserInit((rbuf+4),(rbuf+12)); 
			}	
			else if(temp == -1)
			{
				XBeeJionDisable((rbuf+19),(rbuf+27));
				printf("\033[33m\033[1m已发送拒绝入网指令 \033[0m \n");	
			}
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
		case 1:
			if(*(rbuf+19) == ParkingUsed)
			{	
				printf("\033[33m\033[1m有车辆进入停车位\033[0m \n");
				event_report( char_to_int(rbuf+12),en_vehicle_comming);
			}
			else if(*(rbuf+19) == ParkingUnUsed)
			{
				printf("\033[33m\033[1m当前车位状态为空\033[0m \n");
				event_report( char_to_int(rbuf+12),en_vehicle_leave);
			}
			else if(*(rbuf+19) == ParkLockSuccess)
			{	
				printf("\033[33m\033[1m车位锁定成功 \033[0m \n");
				event_report( char_to_int(rbuf+12),en_lock_success);
			}
			else if(*(rbuf+19) == ParkLockFailed)
			{
				printf("\033[33m\033[1m车位锁定失败 \033[0m \n");	
				event_report( char_to_int(rbuf+12),en_lock_failed);
			}			
			else if(*(rbuf+19) == ParkUnlockSuccess)
			{
				printf("\033[33m\033[1m车位解锁成功 \033[0m \n");
				event_report( char_to_int(rbuf+12),en_unlock_success);
			}			
			else if(*(rbuf+19) == ParkUnlockFailed)
			{
				printf("\033[33m\033[1m车位解锁失败 \033[0m \n");
				event_report( char_to_int(rbuf+12),en_unlock_failed);
			}
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
	netadr_s[1] = (uint8)(netadr>>8);
	netadr_s[0] = (uint8)netadr;
	printf("\033[33m发送锁控制指令完成！\n");
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

/***************************************************************
**brief 两个字节合并一个unt16
***************************************************************/
uint16 char_to_int(uint8 *data)
{
	uint16 reval=0;
	reval |= (uint16)*(data);
	reval |= ((uint16)*(data+1) << 8);
	return reval;
}















