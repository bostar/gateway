#include <unistd.h>
#include <stdio.h>
#include "xbee_bsp.h"
#include "xbee_atcmd.h"
#include "xbee_protocol.h"
#include "xbee_routine.h"
#include <pthread.h>
#include <string.h>
#include "xbee_api.h"
#include "server_duty.h"
#include <time.h>

uint8 rbuf[255];
int16 len;
SourceRouterLinkType *pLinkHead=NULL;
uint8 *HeadMidAdr=NULL;
pthread_mutex_t xbee_mutex;
pthread_mutex_t xbee_mutex_test;
//uint8 XBeeCnt=0;
uint8 net_off=0;


void TestPrintf(int8* sss,int16 lens,uint8 *buf)
{
	int loop=0;
	printf("\033[34m数据序列--%s:数据长度--%d; 数据内容:\033[0m\n",sss,lens);
	for( loop = 0;loop < lens;loop ++)
	{
		if(*(buf+3) == 0x88)
		{
			if(loop==5 || loop==6)
				printf("\033[32m%c \033[0m",(int8)buf[loop]);
			else
				printf("0x%02x ",buf[loop]);
		}
		else if(*(buf+3) == 0x90)
		{
			if(loop==15 || loop==16 || loop==17)
				printf("\33[32m%c \033[0m",(int8)buf[loop]);
			else
				printf("0x%02x ",buf[loop]);
		}
		else
			printf("0x%02x ",buf[loop]);
	}
	printf("\n");
}

void xbee_routine_thread(void)
{
	static time_t timep_s = 0;
	time_t timep_c = 0;
	
	pthread_mutex_init(&xbee_mutex,NULL);
	xbee_gpio_init();
	xbee_serial_port_init(); 
	pthread_mutex_lock(&xbee_mutex);
	CreateGatewayNet();
	pthread_mutex_unlock(&xbee_mutex);
	XBeeSetAR(0x01,NO_RES);
while(1)
{ 
	pthread_mutex_lock(&xbee_mutex);
	len = UartRevDataProcess(rbuf);
	if(len)
	{
		//printf("\033[34m收到数据: \033[0m");	
		//TestPrintf("1",len,rbuf);
		switch(rbuf[3])
		{
			case receive_packet:
				if(rbuf[15]=='C' && rbuf[16]=='F' && rbuf[17]=='G')
					XBeeProcessCFG(rbuf);    
				else if(rbuf[15]=='C' && rbuf[16]=='T' && rbuf[17]=='L')
					XBeeProcessCTL(rbuf);
				else if(rbuf[15]=='S' && rbuf[16]=='E' && rbuf[17]=='N')
					XBeeProcessSEN(rbuf);
				else if(rbuf[15]=='O' && rbuf[16]=='T' && rbuf[17]=='A')
				{}
				break;
			case at_command_response:
				if(rbuf[5]=='N' && rbuf[6]=='J')
				{}
				else if(rbuf[5]=='N' && rbuf[6]=='D')
					ProcessND(rbuf);
				break;
			case transmit_status:
				break;
			case route_record_indicator:
				XBeeProcessRoutRcord(rbuf);
				break;
			case modem_status:
				ProcessModState(rbuf);
				break;
			default:
				break;
		}
	}
	time(&timep_c);
	if((timep_c-timep_s)%AR_PER == 0)
	{
		//printf("\033[33m发送 AR \033[0m\n");
		//XBeeSetAR(0,NO_RES);
	}
	if(net_off == 0)
	{
		if(networking_over() == 1)
		{
			net_off = 1;
			//XBeeSendNetOFF(NET_OFF_TIME);
			printf("\033[32m锁已经全部加入网络\033[0m\n");
		}
	}

	pthread_mutex_unlock(&xbee_mutex);	
}
}

#define VERSION "V1.0"

void xbee_routine_thread_test(void)
{
	int8 in_cmd[100];
	int reval;
	xbee_gpio_init();
	xbee_serial_port_init();
	pthread_mutex_init(&xbee_mutex_test,NULL);
	while(1)
	{
		pthread_mutex_lock(&xbee_mutex_test);
		reval = scanf("%s",in_cmd);
		if(strncmp("linklist",in_cmd,strlen("linklist")) == 0)
			LinkPrintf(pLinkHead);
		else if(strncmp("op",in_cmd,strlen("op")) == 0)
		{
			XBeeReadAT("OP");
			sleep(1);
			printf("\033[34m收到数据: \033[0m");	
			TestPrintf("1",len,rbuf);
		}
		else if(strncmp("sc",in_cmd,strlen("sc")) == 0)
		{
			XBeeReadAT("SC");
			sleep(1);
			printf("\033[34m收到数据: \033[0m");	
			TestPrintf("1",len,rbuf);
		}
		else if(strncmp("sh",in_cmd,strlen("sh")) == 0)
		{
			XBeeReadAT("SH");
			sleep(1);
			printf("\033[34m收到数据: \033[0m");	
			TestPrintf("1",len,rbuf);
		}
		else if(strncmp("sl",in_cmd,strlen("sl")) == 0)
		{
			XBeeReadAT("SL");
			sleep(1);
			printf("\033[34m收到数据: \033[0m");	
			TestPrintf("1",len,rbuf);
		}
		else if(strncmp("oi",in_cmd,strlen("oi")) == 0)
		{
			XBeeReadAT("OI");
			sleep(1);
			printf("\033[34m收到数据: \033[0m");	
			TestPrintf("1",len,rbuf);
		}
		else if(strncmp("ch",in_cmd,strlen("ch")) == 0)
		{
			XBeeReadAT("CH");
			sleep(1);
			printf("\033[34m收到数据: \033[0m");	
			TestPrintf("1",len,rbuf);
		}

		else if(strncmp("locknum",in_cmd,strlen("locknum")) == 0)
		{
			if(networking_over() == 0)
				printf("\033[35m锁已经全部加入网络\033[0m\n");
			else
				printf("\033[35m有锁未加入网络\033[0m\n");
		}
		else if(strncmp("version",in_cmd,strlen("version")) == 0)
			printf("\033[35m软件版本 %s\033[0m\n",VERSION);
		else
			printf("\033[35m无效的命令\033[0m\n");
		pthread_mutex_unlock(&xbee_mutex_test);
	} 
}














