#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "uart_raw.h"
#include <unistd.h>
#include "server_duty.h"
#include <pthread.h>
#include "zlg_cmd.h"
#include "menu.h"
#include "zlg_protocol.h"
#include "xbee_routine.h"
#include <sys/timeb.h>
#include "ctl_cmd_cache.h"
#include "ota.h"

//#define __USE_ZM516X__
//#define __USE_XBEE__

#if !defined(__USE_ZM516X__) && !defined(__USE_XBEE__)
#define __USE_ZM516X__
#endif

#if defined(__USE_ZM516X__)
void zigbee_routine_thread(void)
{
    int ret;
    pthread_t id;

    init_zlg_zm516x();
	
    ret=pthread_create(&id,NULL,(void *) communicate_thread, NULL);
    if(ret!=0){
        printf ("Create communicate_thread error!n");
    }  

    ret=pthread_create(&id,NULL,(void *) menu_thread, NULL);
    if(ret!=0){
        printf ("Create menu_thread error!n");
    }

    while(1)
    {
        usleep(10000);
    }
}
#endif

int main(int argc, char *argv[])
{
    int ret;
    pthread_t id;

#if defined(__USE_ZM516X__)
    if(initCtlCmdCache())
    {
        printf("init ctl cmd cache failed!\r\n");
    }
#endif
    ret=pthread_create(&id,NULL,(void *) server_duty_thread,NULL);
    if(ret!=0){
        printf ("Create server_duty_thread error!n");
    }
#if defined(__USE_ZM516X__)
    printf("start uart_read_thread...\r\n");
    ret=pthread_create(&id,NULL,(void *) zigbee_routine_thread,NULL);
    if(ret!=0){
        printf ("Create uart_read_thread error!n");
    }
#endif
#if defined(__USE_XBEE__)
    printf("start xbee_routine_thread...\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread error!n");
    }
#endif
    printf("start ota_thread...\r\n");
    ret=pthread_create(&id,NULL,(void *) ota_thread,NULL);
    if(ret!=0){
        printf ("Create ota_thread error!n");
    }

    while(1)
    {
        usleep(10000);
    }

    return 0;
}
