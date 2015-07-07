#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "uart_raw.h"
#include <unistd.h>
#include "listener.h"
#include <pthread.h>
#include "zlg_cmd.h"
#include "menu.h"
#include "zlg_protocol.h"
#include <sys/timeb.h>
#define LEN	1000
unsigned char udp_rcv_buf[LEN];
unsigned char uart_rcv_buf[LEN];

void udp_listen_thread(void)
{
    //int len;
    while(1)
    {
        //len = udp_listen(udp_rcv_buf,LEN);
        //uart_write(udp_rcv_buf,len);
    }
}

void test_cmd_thread(void)
{
//	unsigned char rbuf[255];
//	unsigned char rlen = 0;
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
	{/*
		rlen = ReadComPort(rbuf,255);
		if(rlen)
		{
		printf("rbuf is:%s\r\n",rbuf);
		memset(rbuf,0x0,255);
		}*/
		usleep(10000);	
	}
}

void udp_send_thread(void);
void uart_read_thread(void)
{
 //   unsigned char destAddr[2];
//    int ret;
 //   pthread_t id;
//    read_local_cfg();
	init_zlg_zm516x();
/*    ret=pthread_create(&id,NULL,(void *) udp_send_thread,NULL);
    if(ret!=0){
        printf ("Create udp_send_thread error!n");
    }*/
    
    int ret2;
    pthread_t id2;
    ret2=pthread_create(&id2,NULL,(void *) test_cmd_thread,NULL);
    if(ret2!=0){
        printf ("Create test_cmd_thread error!n");
    }
    while(1)
    {
        /*destAddr[0] = 0x00;
        destAddr[1] = 0x01;
        send_data_to_remote_node(destAddr,"Call 0x0001\r\n",13);
        printf("Call 0x0001\r\n");
        usleep(1000000);
        destAddr[0] = 0x00;
        destAddr[1] = 0x02;
        send_data_to_remote_node(destAddr,"Call 0x0002\r\n",13);
        printf("Call 0x0002\r\n");*/
        usleep(1000);
    }
}

void swap(unsigned char len,unsigned char *array)
{
    int k;
    unsigned char temp;
    for (  k = 0 ; k < ( len/ 2 ) ; k++ ) /*将数组对称位置的元素对调*/
    {
        temp = array[k];
        array[k] = array[len-k-1];
        array[len-k-1] = temp;
    }
}
void udp_send_thread(void)
{
    unsigned char buf[200];
    int i;
    int len; 
    int netSize;
    int netid;
    struct timeb tp;
    unsigned char *bytemap;
    static unsigned char byteold = 0;
    unsigned short destAddr;

    ftime(&tp);
    memcpy(buf,"size",4);
    memcpy(&buf[4],stDevInfo.devLoacalIEEEAddr,8);
    while(1)
    {
        printf("get zigbee node num\r\n");
		udp_send_to_server(12,buf);
        printf("send over\r\n");
        usleep(1000000);
        memset(udp_rcv_buf,0,LEN);
        
        if((len = udp_listen(udp_rcv_buf,LEN)) <= 0)
        {
            continue;
        }
        if(memcmp(udp_rcv_buf,"SIZE",4) != 0)
        {
            printf("response error\r\n");
            continue;
        }
        swap(4,&udp_rcv_buf[4]);
        netSize = *(int *)&udp_rcv_buf[4];
        swap(4,&udp_rcv_buf[8]);
        netid = *(int *)&udp_rcv_buf[8];
        if(netSize > 500)
        {
            printf("netsize is too big:%d,%d\r\n",netSize,netid);
            continue;
        }
        bytemap = malloc(netSize);
        memset(bytemap,0,netSize);
        if(bytemap == NULL)
        {
            printf("malloc failed\r\n");
            continue;
        }
        printf("net id/size:%d/%d\r\n",netid,netSize);
        while(1)
        {
            memcpy(buf,"data",4);
            ftime(&tp);
            memcpy(&buf[4],(void *)&tp,8);
            swap(8,&buf[4]);
            memcpy(&buf[12],(void *)&netid,4);
            swap(4,&buf[12]);
            memcpy(&buf[16],&netSize,4);
            swap(4,&buf[16]);
            memcpy(&buf[20],bytemap,netSize);
            udp_send_to_server(20 + netSize,buf);
            usleep(100000);
            memset(udp_rcv_buf,0,LEN);
            while(1)
            {
                len = udp_listen(udp_rcv_buf,LEN);
                if(memcmp(udp_rcv_buf,"TALL",4) != 0)
                {
                    continue;
                }
                memcpy(bytemap,&udp_rcv_buf[20],netSize);
                for(i=0;i<netSize;i++)
                {
                    if(i == 0)
                    {
                        if(byteold != bytemap[i])
                        {
                            byteold = bytemap[i];
                            destAddr = 0x0001;
                            send_data_to_remote_node(destAddr,(unsigned char *)"Call 0x0001\r\n",13);
                            printf("Call 0x0001\r\n");
                        }
                    }
                    printf("%d ",bytemap[i]);
                }
                printf("\r\n");
            }
            usleep(5000000);
        }
    }
}

int main(int argc, char *argv[])
{
    int ret;
    pthread_t id;
    /*if (argc != 3)
    {
        printf ("Please enter server ip and port\n");
        return (0);
    }*/
    /* Fill the socket address struct */
    //uart_init();
//    udp_init();
    /*ret=pthread_create(&id,NULL,(void *) udp_listen_thread,NULL);
    if(ret!=0){
        printf ("Create udp_listen_thread error!n");
    }*/
	printf("start uart_read_thread...\r\n");
    ret=pthread_create(&id,NULL,(void *) uart_read_thread,NULL);
    if(ret!=0){
        printf ("Create uart_read_thread error!n");
    }
    /*ret=pthread_create(&id,NULL,(void *) udp_send_thread,NULL);
    if(ret!=0){
        printf ("Create udp_send_thread error!n");
    }*/

    while(1)
    {
        //printf("main initial over");
		usleep(10000);
    }

    //uart_exit();
    udp_exit();

    return 0;
}
