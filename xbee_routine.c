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


void TestPrintf(int8* sss,int16 lens,uint8 *buf)
{
	int loop=0;
	if((int8)*(buf+15) == 'S' && (int8)*(buf+16) == 'E' && (int8)*(buf+17) == 'N')
		return;
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
				printf("\033[32m%c \033[0m",(int8)buf[loop]);
			else
				printf("0x%02x ",buf[loop]);
		}
		else if(*(buf+3) == 0xa1 && loop == 3)
		{
			printf("\033[32m0x%02x \033[0m",buf[loop]);
		}
		else if(*(buf+3) == 0x8b && loop == 3)
		{
			printf("\033[35m0x%02x \033[0m",buf[loop]);
		}
		else
			printf("0x%02x ",buf[loop]);
	}
	printf("\n");
}

void xbee_routine_thread(void)
{
	pthread_mutex_init(&xbee_mutex,NULL);
	xbee_gpio_init();
	xbee_serial_port_init(); 
	pthread_mutex_lock(&xbee_mutex);
	CreateGatewayNet();
	pthread_mutex_unlock(&xbee_mutex);
	while(1)
	{ 
		pthread_mutex_lock(&xbee_mutex);
		len = UartRevDataProcess(rbuf);
		if(len)
	 	{
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
					ProcessATRes(rbuf);
					break;
				default:
					break;
			}
		}
		static time_t timep_s = 0;
		time_t timep_c = 0;
		time(&timep_c);
		if((timep_c-timep_s) >= 60 )
		{
			CloseNet(0x01);  //关闭网络
			timep_s = timep_c;
		}
		pthread_mutex_unlock(&xbee_mutex);
	 } 
}

#define VERSION "V1.0"

void xbee_routine_thread_test(void)
{
	int8 in_cmd[100];
	int reval;
	uint8 i;
	
	xbee_gpio_init();
	xbee_serial_port_init();
	pthread_mutex_init(&xbee_mutex_test,NULL);
	while(1)
	{
		pthread_mutex_lock(&xbee_mutex_test);
		
		reval = scanf("%s",in_cmd);
		if(strncmp("list",in_cmd,strlen("list")) == 0)
			LinkPrintf(pLinkHead);
		else if(strncmp("op",in_cmd,strlen("op")) == 0)
		{
			XBeeReadAT("OP");
			sleep(1);
			printf("\033[35m64位panID:\033[0m");
			for(i=0;i<8;i++)
				printf("0x%02x ",CoorInfo.panID64[i]);
			printf("\n");
		}
		else if(strncmp("oi",in_cmd,strlen("oi")) == 0)
		{
			XBeeReadAT("OI");
			sleep(1);
			printf("\033[35m16位panID: \033[0m0x%04x\n",CoorInfo.panID16);
		}
		else if(strncmp("nj",in_cmd,strlen("nj")) == 0)
		{
			XBeeReadAT("NJ");
			sleep(1);
			printf("\033[35m允许入网时间 \033[0m0x%02x\n",CoorInfo.nj);
		}
		else if(strncmp("ch",in_cmd,strlen("ch")) == 0)
		{
			XBeeReadAT("CH");
			sleep(1);
			printf("\033[35m网络信道: \033[0m0x%04x\n",CoorInfo.channel);
		}
		else if(strncmp("check",in_cmd,strlen("check")) == 0)
		{
			if(networking_over() == 1)
				printf("\033[35m全部锁已入网\033[0m\n");
			else if(networking_over() == 0)
				printf("\033[35m有锁未入网\033[0m\n");
		}
		else if(strncmp("version",in_cmd,strlen("version")) == 0)
			printf("\033[35m软件版本 %s\033[0m\n",VERSION);
		else
			printf("\033[35m无效的命令\033[0m\n");
		pthread_mutex_unlock(&xbee_mutex_test);
	} 
}














