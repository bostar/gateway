#include <unistd.h>
#include <stdio.h>
#include "xbee_bsp.h"
#include "xbee_atcmd.h"
#include "xbee_protocol.h"
#include "xbee_routine.h"
#include <pthread.h>
#include <string.h>

uint8 rbuf[255];
int16 len;
SourceRouterLinkType *pLinkHead=NULL;
uint8 *HeadMidAdr=NULL;
//uint8 XBeeCnt=0;

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
	xbee_gpio_init();
	xbee_serial_port_init(); 
	CreateGatewayNet();
while(1)
{
	len = UartRevDataProcess(rbuf);
	if(len)
	{
		printf("\033[34m收到数据: \033[0m");	
		TestPrintf("1",len,rbuf);
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
	static uint16 IdleCnt = 0;
	IdleCnt++; 
	if(IdleCnt%10 == 0 || IdleCnt == 1)
	{
		XBeeReadAT("ND");    
		XBeeReadAT("NC");
		XBeeReadAT("OP");
		XBeeReadAT("NJ");
		XBeeReadAT("SC");
	}        
	XBeeSetAR(10,NO_RES);
	printf("\033[32m已存储节点数量%d\033[0m\n",LinkLenth(pLinkHead)-1);
	//LinkPrintf(pLinkHead);
}
}

void xbee_routine_thread_test(void)
{
	int8 in_cmd[100];
	int reval;
	
	xbee_gpio_init();
	xbee_serial_port_init();
	while(1)
	{
		reval = scanf("%s",in_cmd);
		if(strncmp("asd",in_cmd,strlen("asd")) == 0)
			printf("正确\n");
		else 
			printf("错误\n"); 
	} 
}














