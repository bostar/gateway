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
#define LEN	1000

//#define __USE_ZM516X__
//#define __USE_XBEE__

#if !defined(__USE_ZM516X__) && !defined(__USE_XBEE__)
#define __USE_ZM516X__
#endif

#if defined(__USE_ZM516X__)
unsigned char uart_rcv_buf[LEN];
void test_cmd_thread(void)
{
    int ret,ret2;
    pthread_t id,id2;
	
	
    ret=pthread_create(&id,NULL,(void *) menu_thread, NULL);
    if(ret!=0){
        printf ("Create menu_thread error!n");
    }
    ret2=pthread_create(&id2,NULL,(void *) communicate_thread, NULL);
    if(ret2!=0){
        printf ("Create communicate_thread error!n");
    }  
	
    while(1)
    {
        usleep(10000);	
    }
}

void uart_read_thread(void)
{
    init_zlg_zm516x();
    
    int ret2;
    pthread_t id2;
    ret2=pthread_create(&id2,NULL,(void *) test_cmd_thread,NULL);
    if(ret2!=0){
        printf ("Create test_cmd_thread error!n");
    }
    while(1)
    {
        usleep(1000);
    }
}
#endif
int main(int argc, char *argv[])
{
    int ret;
    pthread_t id;
	if(initCtlCmdCache())
		printf("init ctl cmd cache failed!\r\n");
    ret=pthread_create(&id,NULL,(void *) server_duty_thread,NULL);
    if(ret!=0){
        printf ("Create server_duty_thread error!n");
    }
#if defined(__USE_ZM516X__)
    printf("start uart_read_thread...\r\n");
    ret=pthread_create(&id,NULL,(void *) uart_read_thread,NULL);
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

    while(1)
    {
        usleep(10000);
    }

    return 0;
}
