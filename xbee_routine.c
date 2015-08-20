
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
uint8 IdleCnt=0;
//uint8 XBeeCnt=0;

void TestPrintf(int8* sss,int16 lens,uint8 *buf)
{
   	int loop=0;
	printf("serialport recv data %s:len = %d;string is:\n",sss,lens);
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
#if 0
		static uint8 i=0;
		uint8 adr[8],rf[4],net_adr[2];
		uint16 netadr;
		if(i==0)
		{
			i++;
			*adr	=	0x00;	*(adr+1)=	0x13;
			*(adr+2)=	0xa2;	*(adr+3)=	0x00;
			*(adr+4)=	0x40;	*(adr+5)=	0xa1;
			*(adr+6)=	0xa6;	*(adr+7)=	0x97;
			netadr	=	0x2ffe;
			*(net_adr)=	0x2f;	*(net_adr+1)=0xfe;
			*(rf)	=	0x11;	*(rf+1)	=	0x12;
			*(rf+2)	=	0x13;	*(rf+3)	=	0x14;
			XBeePutCtlCmd(adr,netadr, 0);
		}
#endif
		len = UartRevDataProcess(rbuf);  
		//len = xbee_serial_port_read(rbuf);
		IdleCnt++;
		printf("  %d \n",IdleCnt);
		if(len)
			TestPrintf("1",len,rbuf);
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
		usleep(1000000);	
	if(len>0)
		printf("**********the end**********\n");		
	//	TheEnd: IdleCnt++;
#if defined _cycle
    }
#endif
}
















