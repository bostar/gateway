
#include <unistd.h>
#include <stdio.h>
#include "xbee_bsp.h"
#include "xbee_atcmd.h"
#include "xbee_protocol.h"
#include "xbee_routine.h"
#include <pthread.h>

#define _cycle

uint8 rbuf[255];
int16 len;
static uint8 NetState=0;
//uint8 XBeeCnt=0;

void TestPrintf(int8* sss,int16 lens,uint8 *buf)
{
   	int loop=0;
	printf("\033[34m\033[1m数据序列--%s:数据长度--%d; 数据内容:\033[0m\n",sss,lens);
	for( loop = 0;loop < lens;loop ++)
		printf("0x%02x ",buf[loop]);
	printf("\n");
}

void xbee_routine_thread(void)
{
    //int loop = 0;
    xbee_gpio_init();
    xbee_serial_port_init();	
#if 1  
	if(NetState != IN_NET)   //创建网络
	{
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
		NetState = IN_NET;
		printf("\n\033[33m组建网络完成！\033[0m\n");
	}
#endif		
#if defined _cycle 
    while(1)
    {
#endif	
		static uint16 IdleCnt=0;
		IdleCnt++;
		printf("\033[36m调用次数%d\033[0m\n",IdleCnt);
		len = UartRevDataProcess(rbuf);  
		if(len)
		{
			printf("\033[34m\033[1m收到数据: \033[0m");	
			TestPrintf("1",len,rbuf);
		}
#if 1
		if(len)
		{
			switch(rbuf[3])
			{
				case receive_packet:
					if(rbuf[15]=='C' && rbuf[16]=='F' && rbuf[17]=='G')
						XBeeProcessCFG(rbuf);    //处理CFG指令
					if(rbuf[15]=='C' && rbuf[16]=='T' && rbuf[17]=='L')
						XBeeProcessCTL(rbuf);
					if(rbuf[15]=='S' && rbuf[16]=='E' && rbuf[17]=='N')
						XBeeProcessSEN(rbuf);
					if(rbuf[15]=='O' && rbuf[16]=='T' && rbuf[17]=='A')
					{}
					break;
				case route_record_indicator:
					
					break;
				case at_command_response:
					if(rbuf[5]=='N' && rbuf[6]=='J')
        			{}
        			if(rbuf[5]=='S' && rbuf[6]=='H')
        			{}
      				if(rbuf[5]=='S' && rbuf[6]=='L')
        			{}
        			if(rbuf[5]=='M' && rbuf[6]=='Y')
        			{}
					break;
				case transmit_status:
					break;
				case modem_status:
					break;
				default:
					break;
			}
			len = 0;
		}  
#endif
#if 0
		XBeeReadAT("NJ");
		usleep(1000000);
#endif	
		if(len>0)
			printf("**********the end**********\n");		
#if defined _cycle
    }
#endif
}
















