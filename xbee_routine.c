//#include "xbee_api.h"
//#include "xbee_protocol.h"
#include <unistd.h>
#include <stdio.h>
#include "xbee_bsp.h"
#include "xbee_atcmd.h"

#define _cycle

unsigned char rbuf[255];
int len;
void xbee_routine_thread(void)
{
    int loop = 0;
    xbee_gpio_init();
    xbee_serial_port_init();	
#if defined _cycle 
    while(1)
    {
#endif
        //printf("%s\r\n",__func__);
        XBeePanID();
        usleep(1000000);
        len = xbee_serial_port_read(rbuf);
        printf("serialport recv data 1:len = %d;string is:\r\n",len);
        for(loop = 0;loop < len;loop ++)
        {
            printf("0x%02x ",rbuf[loop]);
        }
        printf("\r\n");

        XBeePanID();
        usleep(1000000);
        len = xbee_serial_port_read(rbuf);
        printf("serialport recv data 2:len = %d;string is:\r\n",len);
        for(loop = 0;loop < len;loop ++)
        {
            printf("0x%02x ",rbuf[loop]);
        }
        printf("\r\n");
#if defined _cycle
    }
#endif
}

