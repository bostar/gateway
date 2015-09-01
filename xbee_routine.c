#include <unistd.h>
#include <stdio.h>
#include "xbee_bsp.h"
#include "xbee_atcmd.h"
#include "xbee_protocol.h"
#include "xbee_routine.h"
#include <pthread.h>


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
		printf("0x%02x ",buf[loop]);
	printf("\n");
}

void xbee_routine_thread(void)
{
    xbee_gpio_init();
    xbee_serial_port_init();	 
	uint8 _i,_adr[8];
	for(_i=0;_i<8;_i++)
		_adr[_i] = 0;
	pLinkHead = CreatRouterLink(_adr,0,HeadMidAdr,0);
	XBeeCreateNet();
	SendXBeeReadAIAgain : XBeeReadAI();  
	usleep(1000000);
	len = UartRevDataProcess(rbuf);	
	while(len != 10 || rbuf[3] != 0x88 || rbuf[5] != 'A' || rbuf[6] != 'I' || rbuf[7] != 0 )
	{
		usleep(1000000);
		len = UartRevDataProcess(rbuf);
	}
	if(rbuf[8]!=0)
	{
		XBeeCreateNet();
		goto SendXBeeReadAIAgain;
	}
	//设置睡眠参数
	XBeeSetSP(100,NO_RES);
	printf("\n\033[33m组建网络完成！\033[0m\n");
while(1)
{
	static uint16 IdleCnt=0;
	IdleCnt++;
	printf("\033[36m调用次数%d\033[0m\n",IdleCnt);
	printf("\033[32m已存储节点数量%d\033[0m\n",LinkLenth(pLinkHead));
	LinkPrintf(pLinkHead);
	len = UartRevDataProcess(rbuf);  
	if(len)
	{
		printf("\033[34m收到数据: \033[0m");	
		TestPrintf("1",len,rbuf);
	}
	if(len)
	{
		switch(rbuf[3])
		{
			case receive_packet:
				if(rbuf[15]=='C' && rbuf[16]=='F' && rbuf[17]=='G')
					XBeeProcessCFG(rbuf);    //处理CFG指令
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
				break;
			default:
				break;
		}
		len = 0;
	} 
	if(IdleCnt%10 == 0 || IdleCnt == 1)
	{
		XBeeReadAT("ND");    
		XBeeReadAT("NC");  
	}       
	XBeeSetAR(10,NO_RES);
} 	
}

void xbee_routine_thread_test(void)
{
	xbee_gpio_init();
    xbee_serial_port_init();


}














