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
#include "xbee_api.h"
#include "xbee_atcmd.h"
#include "xbee_bsp.h"
#include "xbee_routine.h"

/******************************************************
**uart receive and check
******************************************************/
int16 UartRevDataProcess(uint8* UartRevBuf)
{
	int16 UartRevLen;
	uint16 DataLen,i;
	uint8 checksum,temp;

	UartRevLen = ReadComPort(UartRevBuf,1);
	if(UartRevLen == 0)
		return 0;
	if(UartRevBuf[0] != 0x7E)
		return 0;
	UartRevLen = ReadComPort(UartRevBuf+1,2);
	if(UartRevLen < 2)
		return 0;
	temp = UartRevBuf[1];
	i = (uint16)temp << 8;
	temp = UartRevBuf[2];
	DataLen = i + (uint16)UartRevBuf[2];
	UartRevLen = ReadComPort(UartRevBuf+3,DataLen+1);
	checksum = XBeeApiChecksum(UartRevBuf+3,DataLen); //校验数据
	if(checksum != UartRevBuf[DataLen+3])
		return 0;
	else
		return DataLen+4;
}
/*******************************************************
**brief process CFG API
*******************************************************/
void XBeeProcessCFG(uint8 *rbuf)
{
	int16 temp;
	switch(*(rbuf+18))
	{
		case net_request:
			temp = get_local_addr(rbuf+12,rbuf+4);
			if(temp == 0)	//属于该网络,允许加入网络
			{
				XBeeJionEnable((rbuf+4),(rbuf+12));
				//printf("\033[33m\033[1m已发送允许入网指令 \033[0m \n");
				set_node_online(rbuf+4);
				//printf("\033[33m\033[1m已将锁加入网络 \033[0m \n");
			}
			else if(temp == -1)
			{
				XBeeJionDisable((rbuf+4),(rbuf+12));
				printf("\033[33m\033[1m已发送拒绝入网指令 \033[0m \n");
				DeleteNode(pLinkHead,FindMacAdr(pLinkHead,rbuf+4));//删除节点
			}
			break;
		default:
			break;
	}
	return;
}
/*******************************************************
**brief  process CTL API
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
**brief process SEN API
*******************************************************/
void XBeeProcessSEN(uint8 *rbuf)
{
	switch(*(rbuf+18))
	{
		case 1:
			if(*(rbuf+19) == ParkingUsed)
			{
				//printf("\033[33m\033[1m当前车位有车辆\033[0m \n");
				event_report( char_to_int(rbuf+12),en_vehicle_comming);
			}
			else if(*(rbuf+19) == ParkingUnUsed)
				{ 
					//printf("\033[33m\033[1m当前车位为空\033[0m \n");
					event_report( char_to_int(rbuf+12),en_vehicle_leave);
				}
			else if(*(rbuf+19) == ParkLockSuccess)
				{ 
					//printf("\033[33m\033[1m车位锁定成功 \033[0m \n");
					event_report( char_to_int(rbuf+12),en_lock_success);
				}
			else if(*(rbuf+19) == ParkLockFailed)
				{
					//printf("\033[33m\033[1m车位锁定失败 \033[0m \n");	
					event_report( char_to_int(rbuf+12),en_lock_failed);
				} 
			else if(*(rbuf+19) == ParkUnlockSuccess)
				{ 
					//printf("\033[33m\033[1m车位解锁成功 \033[0m \n");
					event_report( char_to_int(rbuf+12),en_unlock_success);
				}
			else if(*(rbuf+19) == ParkUnlockFailed)
				{ 
					//printf("\033[33m\033[1m车位解锁失败 \033[0m \n");
					event_report( char_to_int(rbuf+12),en_unlock_failed);
				}
			break;
		case bat_event:
			break;
		default:
			break;
	return;
	}
}
/*************************************************
**brief  record source route path
**       保存经过路由的终端路径
*************************************************/
void XBeeProcessRoutRcord(uint8 *rbuf)
{
	uint16 target_adr=0;
	SourceRouterLinkType *p,*pS;

	if(get_local_addr(rbuf+12,rbuf+4) == 0)
	{
		target_adr |= (uint16)*(rbuf+13);
		target_adr |= (((uint16)*(rbuf+12)) << 8);
		pS = CreatRouterLink(rbuf+4,target_adr,(rbuf+16),*(rbuf+15));
		p = FindMacAdr(pLinkHead,rbuf+4);
		if(p == NULL)
		{
			AddData(pLinkHead,pS);
			//printf("\033[33m新的锁终端路径加入列表...路由节点\033[0m\n");
			return ;
		}
		switch(compareNode(p,pS))
		{
			case 0:
				free(pS);
				pS = NULL;
				//printf("\033[33m锁终端路径已存在...路由节点\033[0m\n");
				break;
			case 1:
				DeleteNode(pLinkHead,p);
				AddData(pLinkHead,pS);
				//printf("\033[33m更新锁终端路径...路由节点\033[0m\n");
				break;
			case 2:
				DeleteNode(pLinkHead,p);
				AddData(pLinkHead,pS);
				//printf("\033[33m新的锁终端路径加入列表...路由节点\033[0m\n");
				break;
			default:
				break;
		} 
	}
	return;
}
/*************************************************
**brief process mode_status
*************************************************/
void ProcessModState(uint8 *rbuf)
{
	if(*(rbuf+4) == 6)
		printf("\032[33mCoordinator Started\033[0m\n");
	else if(*(rbuf+4) == 0)
		printf("\032[33mHardware reset\033[0m\n");
	else if(*(rbuf+4) == 1)
		printf("\032[33mWatchdog timer reset\033[0m\n");
	else if(*(rbuf+4) == 2)
		printf("\032[33mJoined network033[0m\n");
	else if(*(rbuf+4) == 3)
		printf("\032[33mDisassociated\033[0m\n");
	else if(*(rbuf+4) == 7)
		printf("\032[33mNetwork security key was updated\033[0m\n");
	return;
}
/*************************************************
**brief process AT command response
*************************************************/
void ProcessATRes(uint8 *rbuf)
{
	uint8 i;
	if(*(rbuf+7) != 0)
		return;
	if(*(rbuf+5) == 'O' && *(rbuf+6) == 'I')
	{	
		CoorInfo.panID16 = 0;
		CoorInfo.panID16 |= ((uint16)*(rbuf+8))<<8;
		CoorInfo.panID16 |= (uint16)*(rbuf+9);
	}
	else if(*(rbuf+5) == 'O' && *(rbuf+6) == 'P')
	{
		for(i=0;i<8;i++)
			CoorInfo.panID64[i] = *(rbuf+8+i);
	}
	else if(*(rbuf+5) == 'C' && *(rbuf+6) == 'H')
		CoorInfo.channel = *(rbuf+8);
	else if(*(rbuf+5) == 'N' && *(rbuf+6) == 'J')
	{
		CoorInfo.nj = *(rbuf+8);
	}
	return;
}
/*************************************************
**brief process ND AT command response
**      添加直接隶属协调器的终端到列表，添加列表中终端类型
*************************************************/
void ProcessND(uint8 *rbuf)
{
	uint16 target_adr=0;
	SourceRouterLinkType *p,*pS;
	if(*(rbuf+20)==0 && *(rbuf+21)==0 )  
	{
		target_adr |= (((uint16)*(rbuf+8)) << 8);
		target_adr |= (uint16)*(rbuf+9);
		pS = CreatRouterLink(rbuf+10,target_adr,rbuf,0);
		p = FindMacAdr(pLinkHead,rbuf+10);
		if(p == NULL)
		{
			AddData(pLinkHead,pS);
			printf("\033[33m新的锁终端路径加入列表...直属节点\033[0m\n");
			return ;
		}
		switch(compareNode(p,pS))
		{ 
			case 0:
				free(pS);
				pS = NULL;
				//printf("\033[33m锁终端路径已存在...直属节点\033[0m\n");
				break;
			case 1:
				DeleteNode(pLinkHead,p);
				AddData(pLinkHead,pS);
				printf("\033[33m更新锁终端路径...直属节点\033[0m\n");
				break;
			case 2:
				AddData(pLinkHead,pS);
				printf("\033[33m新的锁终端路径加入列表...直属节点\033[0m\n");
				break;
			default:
				break;
		}
	}
	p = FindMacAdr(pLinkHead,rbuf+10);
	if(p != NULL)
		p->dev_type = *(rbuf+22);  //1 is router  2 is end device  save device type
		//XBeeSendDevType();
	return;
}
/*************************************************
**brief 向router发送限时加入网络命令
*************************************************/
int16 XBeeSendNetOFF(uint8 time)
{
	uint8 data[5];
	
	data[0]  =  'C';
	data[1]  =  'F';
	data[2]  =  'G';
	data[3]  = 0X00;
	data[4]  = time;
	return XBeeBoardcastTrans(data,5,NO_RES);
}
/**************************************************
**brief 允许入网
**************************************************/
int16 XBeeJionEnable(uint8 *ieeeadr,uint8 *netadr)
{
	uint8 data[5];
	uint16 target_adr=0;
	SourceRouterLinkType *p=NULL;
	
	target_adr |= (uint16)*(netadr+1);
	target_adr |= (((uint16)*(netadr+0)) << 8);
	data[0]  =  'C';
	data[1]  =  'F';
	data[2]  =  'G';
	data[3]  =  0x02;
	data[4]  =  0x01;
	p = FindMacAdr(pLinkHead,ieeeadr);
	if(p != NULL)
		XBeeCreatSourceRout(p->mac_adr,p->target_adr,p->num_mid_adr,p->mid_adr);
	return XBeeTransReq(ieeeadr,netadr,Default,data,5,NO_RES);
}
/**************************************************
**brief 禁止入网
**************************************************/
int16 XBeeJionDisable(uint8 *ieeeadr,uint8 *netadr)
{
	uint8 data[5];
	SourceRouterLinkType *p;
	data[0]  =  'C';
	data[1]  =  'F';
	data[2]  =  'G';
	data[3]  =  0x02;
	data[4]  =  0x00;
	p = FindMacAdr(pLinkHead,ieeeadr);
	if( p != NULL)
		XBeeCreatSourceRout(p->mac_adr,p->target_adr,p->num_mid_adr,p->mid_adr);
	return XBeeTransReq(ieeeadr,netadr,Default,data,5,NO_RES);
}
/*******************************************************
**brief 发送恢复出厂设置指令
*******************************************************/
int16 XBeeSendFactorySettingCmd(uint8 *ieeeadr,uint8 *netadr)
{
	uint8 data[4];
	SourceRouterLinkType *p;
	data[0]  =  'C';
	data[1]  =  'F';
	data[2]  =  'G';
	data[3]  =  0x03;
	p = FindMacAdr(pLinkHead,ieeeadr);
	if(p != NULL)
		XBeeCreatSourceRout(p->mac_adr,p->target_adr,p->num_mid_adr,p->mid_adr);
	return XBeeTransReq(ieeeadr,netadr,Default,data,4,NO_RES);
}
/*******************************************************
**brief 发送设置router的NJ
*******************************************************/
int16 XBeeSendSetNJ(uint8 *mac_adr,uint8 time)
{
	
	return 0;
}
/*******************************************************
**brief 发送设备类型
*******************************************************/
int16 XBeeSendDevType(uint8 *mac_adr,uint8 *net_adr)
{
	uint8 data[5];
	uint16 target_adr=0;
	SourceRouterLinkType *p;
	
	p = FindMacAdr(pLinkHead,mac_adr);
	if(p == NULL)
	{
		printf("\033[33mERROR!!！节点不存在\033[0m\n");
		return 0;
	}
	target_adr |= (uint16)*(net_adr+1);
	target_adr |= (((uint16)*(net_adr+0)) << 8);
	data[0]  =  'C';
	data[1]  =  'F';
	data[2]  =  'G';
	data[3]  =  0x04;
	if(p->dev_type == 0x01)
		data[4]  =  0x01;
	else if(p->dev_type == 0x02)
		data[4]	=	0x02;
	
	//if(p->num_mid_adr != 0)
		XBeeCreatSourceRout(p->mac_adr,p->target_adr,p->num_mid_adr,p->mid_adr);
	return XBeeTransReq(mac_adr,net_adr,Default,data,5,NO_RES);
}
/*******************************************************
**brief 终端控制命令
**param	ieeeadr	目标的8字节地址
		netadr	目标的网络地址
		lockstate 0 解锁
				  1 锁定
*******************************************************/
int16 XBeePutCtlCmd(uint8 *ieeeadr,uint16 netadr,uint8 lockstate)
{
	uint8 data[5];
	SourceRouterLinkType *p;

	data[0]  =  'C';	
	data[1]  =  'T';
	data[2]  =  'L';
	data[3]  =  0x00;
	data[4]  =  lockstate;
	uint8 netadr_s[2];
	netadr_s[1] = (uint8)(netadr>>8);
	netadr_s[0] = (uint8)netadr;
	p = FindMacAdr(pLinkHead,ieeeadr);
	if(p != NULL)
		XBeeCreatSourceRout(p->mac_adr,p->target_adr,p->num_mid_adr,p->mid_adr);
	//printf("\033[33m发送锁控制指令完成！\033[0m\n");
	return XBeeTransReq(ieeeadr,netadr_s,Default,data,5,NO_RES);
}
/**************************************************************
**brief 标定senser初始值
**************************************************************/
int16 XBeeSendSenserInit(uint8 *ieeeadr,uint8 *net_adr)
{
	uint8 data[4];
	SourceRouterLinkType *p;
	data[0]		=	'S';
	data[1]		=	'E';
	data[2]		=	'N';
	data[3]		=	0;
	p = FindMacAdr(pLinkHead,ieeeadr);
	if(p != NULL)
		XBeeCreatSourceRout(p->mac_adr,p->target_adr,p->num_mid_adr,p->mid_adr);
	return XBeeTransReq(ieeeadr,net_adr,Default,data,4,NO_RES);
}
/**************************************************************
**brief 周期发送AR
**************************************************************/
void SendAR(uint8 perid)
{
	static time_t timep_s = 0;
	time_t timep_c = 0;
	time(&timep_c);
	if((timep_c-timep_s) >= perid || timep_s == 0)
	{
		//XBeeSetAR(0,NO_RES);
		timep_s = timep_c;
	}
	return;
}
/**************************************************************
**brief 发送关闭网络时间
**************************************************************/
void CloseNet(uint8 time)
{
	static uint8 net_off = 0;
	if(networking_over() == 0)
	{
		net_off = 0;
		XBeeSendNetOFF(0xff);
		XBeeSetNJ(0xff,NO_RES);
	}
	if(net_off == 0 && networking_over() == 1)
	{
		net_off = 1;
		XBeeSendNetOFF(time);
		XBeeSetNJ(time,NO_RES);
		printf("\033[32m锁已经全部加入网络\033[0m\n");
	}
}
/**************************************************************
**brief 配置组网
**************************************************************/
void CreateGatewayNet(void)
{
	uint8 _i,_adr[8],state=0;
	uint8 len,rbuf[128];
	for(_i=0;_i<8;_i++)
		_adr[_i] = 0;
	pLinkHead = CreatRouterLink(_adr,0,HeadMidAdr,0);    //创建路由路径链表
	XBeeCreateNet();
	state = 1;
	while(state != 0)
	{
		XBeeReadAI();
		usleep(2000);
		len = UartRevDataProcess(rbuf);
		if(len > 0 )
		{
			if(rbuf[3] == 0x88 || rbuf[5] == 'A' || rbuf[6] == 'I' || rbuf[7] == 0)
			{
				state = rbuf[8];
			}
		}
	}
	XBeeSetSP(200,NO_RES);	//设置睡眠参数
	CoorInfo.NetState = 1;
	printf("\n\033[33m组建网络完成！\033[0m\n");
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
/****************************************************************
**brief 打印当前时间
****************************************************************/
void printf_local_time(void)
{
	time_t now;    //实例化time_t结构
	struct tm  *timenow;    //实例化tm结构指针
	
	timenow = localtime(&now);//localtime函数把从time取得的时间now换算成你电脑中的时间(就是你设置的地区)
	printf("Local time is %s\n",asctime(timenow));//asctime函数把时间转换成字符，通过printf()函数输出
}













