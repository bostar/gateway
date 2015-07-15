#include "xbee_api.h"
#include "xbee_protocol.h"
#include <unistd.h>
#include <stdio.h>
void xbee_routine_thread(void)
{
    while(1)
    {
        printf("%s\r\n",__func__);
        usleep(1000000);
    }
}
