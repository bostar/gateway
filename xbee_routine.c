
#include <unistd.h>
#include <stdio.h>
#include "xbee_bsp.h"
#include "xbee_atcmd.h"
#include "xbee_protocol.h"
#include "xbee_routine.h"
#include <pthread.h>

#define _cycle
//#define CREATE_NET
#define _TEST

uint8 rbuf[255];
int16 len;
//static uint8 NetState=0;
//uint8 XBeeCnt=0;

void TestPrintf(int8* sss,int16 lens,uint8 *buf)
{
   	int loop=0;
	printf("\033[34m\033[1m数据序列 %s:数据长度 = %d; 数据内容:\033[0m\n",sss,lens);
	for( loop = 0;loop < lens;loop ++)
		printf("0x%02x ",buf[loop]);
	printf("\n");
}

void xbee_routine_thread(void)
{
    //int loop = 0;
    xbee_gpio_init();
    xbee_serial_port_init();	


#if defined CREATE_NET   
	if(NetState != IN_NET)   //创建网络
	{
		XBeeCreateNet();
		SendXBeeReadAIAgain : XBeeReadAI(RES);  
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
		
		NetState = IN_NET;
	}
#endif		
#if defined _cycle 
    while(1)
    {
#endif	
		static uint16 IdleCnt=0;

		len = UartRevDataProcess(rbuf);  
		//len = xbee_serial_port_read(rbuf);
		IdleCnt++;
		printf("\033[36m调用次数%d\033[0m\n",IdleCnt);
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
				case 0x90:
					if(rbuf[15]=='C' && rbuf[16]=='F' && rbuf[17]=='G')
						XBeeProcessCFG(rbuf);    //处理CFG指令
					if(rbuf[15]=='C' && rbuf[16]=='T' && rbuf[17]=='L')
						XBeeProcessCTL(rbuf);
					if(rbuf[15]=='S' && rbuf[16]=='E' && rbuf[17]=='N')
						XBeeProcessSEN(rbuf);
					if(rbuf[15]=='O' && rbuf[16]=='T' && rbuf[17]=='A')
					{}
					len = 0;
					break;
				case 0x88:
					if(rbuf[5]=='N' && rbuf[6]=='J')
        			{}
        			if(rbuf[5]=='S' && rbuf[6]=='H')
        			{}
      				if(rbuf[5]=='S' && rbuf[6]=='L')
        			{}
        			if(rbuf[5]=='M' && rbuf[6]=='Y')
        			{}
					len = 0;
					break;
				case 0x8b:
					break;
				case 0x8a:
					break;
				default:
					len = 0;
					break;
			}
		}  
#endif
		//usleep(200000);	
		if(len>0)
			printf("**********the end**********\n");		
#if defined _cycle
    }
#endif
}
















