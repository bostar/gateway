/*****************************************************************************************************
**name	xbee_protocol.c
**brief	主要包括xbee通讯协议函数/上报的事件处理函数
**
*****************************************************************************************************/
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
//#include "xbee_api.h"
#include "xbee_atcmd.h"
#include "xbee_bsp.h"
#include "xbee_routine.h"
#include "pthread.h"
#include "xbee_config.h"
#include "stdbool.h"

/*******************************************************
**brief process CFG API
*******************************************************/
void XBeeProcessCFG(uint8 *rbuf)
{
	int16 temp;
	SourceRouterLinkType *p;
	switch(*(rbuf+18))
	{
		case net_request:
#if defined __XBEE_TEST_LAR_NODE__
			temp = 0;
			if(temp == 0)  //属于该网络,允许加入网络
			{
				for(temp=0;temp<8;temp++)
					printf("%02x ",*(rbuf+4+temp));
				printf("\n");
				XBeeJionEnable((rbuf+4),(rbuf+12));
				pthread_mutex_lock(&mutex02_pLinkHead);
				p = FindMacAdr(pLinkHead,rbuf+4); 
				if(p == NULL)
					AddData(pLinkHead,CreatNode(rbuf+4,rbuf+12));
				else
				{
					p->target_adr = 0;
					p->target_adr |= (uint16)*(rbuf+13);
					p->target_adr |= ((uint16)*(rbuf+12))<<8;
				}
				pthread_mutex_unlock(&mutex02_pLinkHead);
				XBeeSetAR(0,RES);
			}
#else
			temp = get_local_addr(rbuf+12,rbuf+4);
			if(temp == 0)	//属于该网络,允许加入网络
			{
				XBeeJionEnable((rbuf+4),(rbuf+12));
				printf("\033[33m\033[1ma locker has jioned the park net...\033[0m \n");
				set_node_online(rbuf+4);
				XBeeSetAR(0,RES);
				//printf("\033[33m\033[1m已将锁加入网络 \033[0m \n");
			}
#endif
			else if(temp == -1)
			{
				XBeeJionDisable((rbuf+4),(rbuf+12));
				printf("\033[31m\r\nprevent a locker jioned	the park net...\033[0m \r\n");
				pthread_mutex_lock(&mutex13_pSourcePathList);
				p = FindMacAdr(pSourcePathList,rbuf+4); 
				if(p != NULL)
					DeleteNode(pSourcePathList,FindMacAdr(pSourcePathList,rbuf+4));//删除节点
				pthread_mutex_unlock(&mutex13_pSourcePathList);
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
	get_local_addr(rbuf+12,rbuf+4);
	if(*(rbuf+19) == 0x01)	//传感器事件使能
	{
		if(*(rbuf+20) == ParkingUsed)
		{
			//printf("\033[33m\033[1m当前车位有车辆\033[0m \n");
			event_report( char_to_int(rbuf+12),en_vehicle_comming);
		}
		else if(*(rbuf+20) == ParkingUnUsed)
		{
			//printf("\033[33m\033[1m当前车位为空\033[0m \n");
			event_report( char_to_int(rbuf+12),en_vehicle_leave);
		}
	}
	else
	{
		printf("\033[31m\033[1m传感器事件未上报\033[0m\n");
	}
	if(*(rbuf+21) == 0x01)	//锁状态上报
	{
		switch(*(rbuf+22))
		{
			case ParkLockSuccess:
				//printf("\033[33m\033[1m车位锁定成功 \033[0m \n");
				event_report( char_to_int(rbuf+12),en_lock_success);
				break;
			case ParkLockFailed:
				//printf("\033[33m\033[1m车位锁定失败 \033[0m \n");	
				event_report( char_to_int(rbuf+12),en_lock_failed);
				break;
			case ParkUnlockSuccess:
				//printf("\033[33m\033[1m车位解锁成功 \033[0m \n");
				event_report( char_to_int(rbuf+12),en_unlock_success);
				break;
			case ParkUnlockFailed:
				//printf("\033[33m\033[1m车位解锁失败 \033[0m \n");
				event_report( char_to_int(rbuf+12),en_unlock_failed);
				break;
			default:
				break;
		}
	}
	else
	{
		printf("\033[31m\033[1m锁事件未上报\033[0m\n");
	}
	if(*(rbuf+23) == 0x01)	//电量上报
	{
		
	}
}
/*************************************************
**brief  record source route path
**       保存经过路由的终端路径
*************************************************/
void XBeeProcessRoutRcord(SourceRouterLinkType *p_head , uint8 *rbuf)
{
	uint16 target_adr=0;
	SourceRouterLinkType *p,*pS;

	target_adr = 0;
	target_adr |= (uint16)*(rbuf+13);
	target_adr |= (((uint16)*(rbuf+12)) << 8);
	pS = CreatRouterLink(rbuf+4,target_adr,(rbuf+16),*(rbuf+15));
	//NodePrintf(pS);
	p = FindMacAdr(p_head,rbuf+4);
	if(p != NULL)
	{
		switch(compareNode(p,pS))
		{
			case 0:
				free(pS);
				pS = NULL;
				//printf("\033[33m锁终端路径已存在...路由节点\033[0m\n");
				break;
			case 1:
				DeleteNode(p_head,p);
				AddData(p_head,pS);
				//printf("\033[33m更新锁终端路径...路由节点\033[0m\n");
				break;
			case 2:
				DeleteNode(p_head,p);
				AddData(p_head,pS);
				//printf("\033[33m新的锁终端路径加入列表...路由节点\033[0m\n");
				break;
			default:
				break;
		}
	}
	else
	{
		AddData(p_head,pS);
		//printf("\033[33m新的锁终端路径加入列表...路由节点\033[0m\n");
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
	pthread_mutex_lock(&mutex14_CoorInfo);
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
	else if(*(rbuf+5) == 'I' && *(rbuf+6) == 'D')
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
	else if(*(rbuf+5) == 'A' && *(rbuf+6) == 'R')
	{
		CoorInfo.ar = *(rbuf+8);
	}
	pthread_mutex_unlock(&mutex14_CoorInfo);
	return;
}
/*************************************************
**brief process teansmit state 
*************************************************/
void ProcessTranState(void)
{
	
}
/*************************************************
**brief process ND AT command response
**      添加直接隶属协调器的终端到列表，添加列表中终端类型
*************************************************/
void ProcessND(uint8 *rbuf)
{
	
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
	XBeeBoardcastTrans(data,5,RES);
	return 0;
}
/**************************************************
**brief 允许入网
**************************************************/
int16 XBeeJionEnable(uint8 *ieeeadr,uint8 *netadr)
{
	uint8 data[5];
	uint16 target_adr=0;
	
	target_adr |= (uint16)*(netadr+1);
	target_adr |= (((uint16)*(netadr+0)) << 8);
	data[0]  =  'C';
	data[1]  =  'F';
	data[2]  =  'G';
	data[3]  =  0x02;
	data[4]  =  0x01;
	XBeeUnicastTrans(ieeeadr,netadr,Default,data,5,RES);
	return 0;
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
	XBeeUnicastTrans(ieeeadr,netadr,Default,data,5,RES);
	return 0;
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
	XBeeUnicastTrans(ieeeadr,netadr,Default,data,4,RES);
	return 0;
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
	return 0;
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

	data[0]  =  'C';	
	data[1]  =  'T';
	data[2]  =  'L';
	data[3]  =  0x00;
	data[4]  =  lockstate;
	uint8 netadr_s[2];
	netadr_s[1] = (uint8)(netadr>>8);
	netadr_s[0] = (uint8)netadr;
	printf("\033[31m 控制锁指令\033[0m\r\n");
	XBeeUnicastTrans(ieeeadr,netadr_s,Default,data,5,RES);
	return 0;
}
/**************************************************************
**brief 标定senser初始值
**************************************************************/
int16 XBeeSendSenserInit(uint8 *ieeeadr,uint8 *net_adr)
{
	uint8 data[4];
	data[0]		=	'S';
	data[1]		=	'E';
	data[2]		=	'N';
	data[3]		=	0;
	XBeeUnicastTrans(ieeeadr,net_adr,Default,data,4,RES);
	return 0;
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
		XBeeSetAR(0,NO_RES);
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
/*	if(networking_over() == 0 && net_off == 1)
	{
		net_off = 0;
		XBeeSendNetOFF(0xff);
		XBeeSetNJ(0xff,NO_RES);
		printf("\033[32m网络允许锁重新入网\033[0m\n");
	} */
	if(net_off == 0 && networking_over() == 1)
	{
		net_off = 1;
		XBeeSendNetOFF(time);
		XBeeSetNJ(time,NO_RES);
		printf("\033[32m锁已经全部加入网络\033[0m\n");
	}
}
/**************************************************************
**brief xbee组网
**************************************************************/
void XBeeNetInit(void)
{
	uint8 state=0;
	uint8 rbuf[128];
	uint8 panID[8],i;
	bool status;

	printf("\n\033[33mcreat xbee network ...\033[0m\n");
	xbee_serial_port_init(115200);
	usleep(100000);
	XBeeSendAT("RE");
	usleep(100000);
	LeaveNetwork();
	usleep(300000);
	xbee_serial_port_init(9600);
	usleep(10000);
	XBeeSendAT("RE");
	usleep(10000);
	LeaveNetwork();	
	usleep(300000);
	for(i=0;i<8;i++)
		panID[i] = 0x00;
	XBeeSetPanID(panID,NO_RES);   //设置ID的值
	XBeeSetChannel(SCAN_CHANNEL,NO_RES); //设置信道
	//XBeeSetZS(1,NO_RES);
	XbeeSendAC(NO_RES);
	state = 1;
	while(state != 0)
	{
		pthread_mutex_lock(&mutex09_xbee_other_api_buf);
		status = read_one_package_f_queue(&xbee_other_api_buf , rbuf);
		pthread_mutex_unlock(&mutex09_xbee_other_api_buf);
		if(status == true)
		{
			if(rbuf[3] == 0x88 && rbuf[5] == 'A' && rbuf[6] == 'I' && rbuf[7] == 0)
			{
				state = rbuf[8];
			}
		}
		XBeeReadAI();
		usleep(200000);
	}
	XBeeSetSP(0x0af0,NO_RES);
	XBeeSetSN(10,NO_RES);
	XBeeSetBD(115200);
	XbeeSendAC(NO_RES);
	usleep(100000);
	xbee_serial_port_init(115200);
	XBeeSendWR(NO_RES);
	CoorInfo.NetState = 1;
	printf("\n\033[33mxbee network established！\033[0m\n");
}
/***************************************************************
**brief get mac addr from xbee
**
***************************************************************/
void get_mac(void)
{
	uint8 i=0,rbuf[128],mac_adr[8];
	bool status=false;

	XBeeReadAT("SH");
	while(status == false)
	{
		usleep(1000);
		pthread_mutex_lock(&mutex09_xbee_other_api_buf);
		status = read_one_package_f_queue(&xbee_other_api_buf , rbuf);
		pthread_mutex_unlock(&mutex09_xbee_other_api_buf);
		if(status == true)
		{
			if(rbuf[3] == 0x88 && rbuf[5] == 'S' && rbuf[6] == 'H' && rbuf[7] == 0)
			{
				for(i=0;i<4;i++)
					mac_adr[i] = rbuf[8+i];
			}
			else if(rbuf[3] == 0x88 && rbuf[5] == 'S' && rbuf[6] == 'H' && rbuf[7] != 0)
			{
				XBeeReadAT("SH");
				status = false;
			}
			else
				status = false;
		}
	}
	XBeeReadAT("SL");
	status = false;
	while(status == false)
	{
		usleep(1000);
		pthread_mutex_lock(&mutex09_xbee_other_api_buf);
		status = read_one_package_f_queue(&xbee_other_api_buf , rbuf);
		pthread_mutex_unlock(&mutex09_xbee_other_api_buf);
		if(status == true)
		{
			if(rbuf[3] == 0x88 && rbuf[5] == 'S' && rbuf[6] == 'L' && rbuf[7] == 0)
			{
				for(i=0;i<4;i++)
					mac_adr[4+i] = rbuf[8+i];
			}
			else if(rbuf[3] == 0x88 && rbuf[5] == 'S' && rbuf[6] == 'L' && rbuf[7] != 0)
			{
				XBeeReadAT("SL");
				status = false;
			}
			else
				status = false;
		}
	}
	printf("\033[34mcoor mac addr : ");
	pthread_mutex_lock(&mutex14_CoorInfo);
	for(i=0;i<8;i++)
	{
		CoorInfo.mac_adr[i] = mac_adr[i];
		printf("%02x ",CoorInfo.mac_adr[i]);
	}
	pthread_mutex_unlock(&mutex14_CoorInfo);
	puts("\033[0m");
}
/***************************************************************
**brief get mac addr from CoorInfo
**
***************************************************************/
int get_gateway_mac_addr(unsigned char *macAddr)
{
	uint8 i=0,sum=0;
	pthread_mutex_lock(&mutex14_CoorInfo);
	for(i=0;i<8;i++)
	{
		macAddr[i] = CoorInfo.mac_adr[i];
		sum |= CoorInfo.mac_adr[i];
	}
	pthread_mutex_unlock(&mutex14_CoorInfo);
	if(sum == 0)
		return -1;
	else
		return 0;
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
/*****************************************************************************
**brief	analysis package from a queue
**param	
**reval	
*****************************************************************************/
uint16 read_one_package_f_queue( CircularQueueType* p_cqueue , uint8* buf )
{
	int16 len;
	uint16 DataLen=0,cnt=0;
	uint8 checksum;
	uint8 UartRevBuf[255];
	
	len = read_cqueue(p_cqueue , UartRevBuf , 1);
	//printf("\033[32mlen = %d\033[0m\n",len);
	if(len == 0)
		return 0;
	if(UartRevBuf[0] != 0x7E)
		return 0;
	len = read_cqueue(p_cqueue , UartRevBuf+1 , 2);
	//printf("\033[32mlen = %d\033[0m\n",len);
	if(len < 2)
		return 0;
	DataLen = 0;
	DataLen |= UartRevBuf[2];
	DataLen |= (uint16)UartRevBuf[1]<<8;
	len = read_cqueue(p_cqueue , UartRevBuf+3 , DataLen+1);
	//printf("\033[32mlen = %d\033[0m\n",len);
	if(len < DataLen+1)
		return 0;
	checksum = XBeeApiChecksum(UartRevBuf+3,DataLen); //校验数据
	if(checksum != UartRevBuf[DataLen+3])
		return 0;
	else
	{	
		for(cnt=0;cnt<DataLen+4;cnt++)
			*(buf+cnt) = *(UartRevBuf+cnt);
		return DataLen+4;
	}
	return 0;
}




















