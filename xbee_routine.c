//#include "xbee_api.h"
//#include "xbee_protocol.h"
#include <unistd.h>
#include <stdio.h>
#include "xbee_bsp.h"
#include "xbee_atcmd.h"

#define _cycle

uint8 rbuf[255];
int len;

void _fprintt(int8* sss)
{
   int loop=0;
   usleep(1000000);
   len = xbee_serial_port_read(rbuf);
   printf("serialport recv data %s:len = %d;string is:\r\n",sss,len);
   for(loop = 0;loop < len;loop ++)
   printf("0x%02x ",rbuf[loop]);
   printf("\r\n");
   printf("\r\n");
}

void xbee_routine_thread(void)
{
    //int loop = 0;
    xbee_gpio_init();
    xbee_serial_port_init();	
#if defined _cycle 
    while(1)
    {
#endif
		printf("\r\n");
		XBeeSendNC(RES);
        _fprintt("receive ");
  
        printf("**********the end**********\r\n");
#if defined _cycle
    }
#endif
}

