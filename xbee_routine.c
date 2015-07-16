//#include "xbee_api.h"
//#include "xbee_protocol.h"
#include <unistd.h>
#include <stdio.h>
#include "xbee_bsp.h"
unsigned char rbuf[255];
int len;
void xbee_routine_thread(void)
{
    int loop = 0;
    xbee_gpio_init();
    xbee_serial_port_init();

    while(1)
    {
        //printf("%s\r\n",__func__);
        XBeeOpenBuzzer();
        usleep(1000000);
        len = xbee_serial_port_read(rbuf);
        printf("serialport recv data :len = %d;string is:\r\n",len);
        for(loop = 0;loop < len;loop ++)
        {
            printf("0x%02x ",rbuf[loop]);
        }
        printf("\r\n");
        XBeeCloseBuzzer();
        usleep(1000000);
        printf("serialport recv data :len = %d;string is:\r\n",len);
        for(loop = 0;loop < len;loop ++)
        {
            printf("0x%02x ",rbuf[loop]);
        }
        printf("\r\n");
    }
}
