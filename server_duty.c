#include "listener.h"
#include <unistd.h>
#include "server_duty.h"
#include <string.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <pthread.h>
#include "zlg_protocol.h"
#include <time.h>
#include "zlg_cmd.h"
#include "parking_state_management.h"
static const unsigned char mac_addr[8] = {0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8};
unsigned short freetime = 30;
unsigned short leavetime = 30;
//static const unsigned char mac_addr[8] = {0x00,0x97,0x19,0x2b,0x00,0x15,0x8d,0x00};
extern unsigned char socketisok;

void get_channel_panid(unsigned char* channel,unsigned short*panid)
{
    if((channel == NULL) || (panid == NULL))
    {
        return ;
    }
    *channel = 26;//CH15,20,25,26
    *panid = 0x0001;
    return;
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

void server_duty_thread(void)
{
    static unsigned char wbuf[255],rbuf[500 * 10];
    int len;
    int loop = 0;
    struct timeb tp;
    time_t time_out = time((time_t*)NULL);
    int ret;
    pthread_t id;
    unsigned char gatewaymac[8];
    //static unsigned char ctl;
    tcp_init();
    while(get_gateway_mac_addr(gatewaymac) != 0)usleep(100000);
    printf("get mac addr: ");
    for(loop = 0;loop < 8;loop ++)
    {
        printf("%02x",gatewaymac[loop]);
    }
    printf("\r\n");
cfg:
    memcpy(wbuf,"size",4); // pkg head
    memcpy((unsigned char *)&wbuf[4],(unsigned char *)gatewaymac,8); // mac addr
    while((len = tcp_send_to_server(12,wbuf)) < 12)
    {
        printf("[SERVER]send to server err\r\n");
        usleep(1000000);
    }
    
    usleep(1000000);
    len = tcp_listen(rbuf,15);
    //len = tcp_listen(rbuf,sizeof(rbuf));
    printf("len = %d",len);
    if(memcmp(rbuf,"SIZE",4) != 0)
    {
        printf("[SERVER]read again...\r\n");
        goto cfg;
    }
    swap(4,&rbuf[4]);
    swap(4,&rbuf[8]);
    swap(4,&rbuf[14]);
    set_depot_info(*(int *)(&rbuf[4]),*(int *)(&rbuf[8]),rbuf[12],*(int *)(&rbuf[14]));
    while((pstParkingState = malloc(sizeof(st_parkingState) * *(int *)(&rbuf[8]))) == NULL)
    {
        printf("[SERVER]malloc park_info memory failed!\r\n");
    }
    parking_init();
down:
    usleep(1000000);
    /* download the parking info of this depot */
    len = tcp_listen(rbuf,8 + get_depot_size() * 10);
    if(memcmp("DOWN",rbuf,4) != 0)
    {
        printf("[SERVER]is not download parking info cmd\r\n");
        usleep(1000000);
        goto down;
    }

    for(loop = 0;loop < get_depot_size();loop ++)
    {
        swap(2,&rbuf[8 + loop * 2 + loop * 8]);
        //swap(8,&rbuf[8 + 2 + loop * 2 + loop * 8]);
        parking_id_macaddr_mapping(*(unsigned short *)&rbuf[8 + loop * 2 + loop * 8],
                                   &rbuf[8 + 2 + loop * 2 + loop * 8]);

    }
time:
    len = tcp_listen(rbuf,12);
    if(memcmp("TIME",rbuf,4) != 0)
    {
        printf("[SERVER]is not TIME cmd\r\n");
        usleep(1000000);
        goto time;
    }
    swap(2,&rbuf[8]);
    swap(2,&rbuf[10]);
    freetime = *(unsigned short*)&rbuf[8];
    leavetime = *(unsigned short*)&rbuf[10];
    printf("[SERVER]free time is %d minute\r\n",freetime);


    ret=pthread_create(&id,NULL,(void *) parking_state_check_routin,NULL);
    if(ret!=0){
        printf ("Create parking_state_check_routin error!n");
    }
    /*ret=pthread_create(&id,NULL,(void *) pkg,NULL);
    if(ret!=0){
        printf ("Create parking_state_check_routin error!n");
    }*/

    while(1)
    {
        if(socketisok == 1)
        {
            if((need_to_send_to_sever == 1) || (time((time_t*)NULL) - time_out > 2))
            {
                time_out = time((time_t*)NULL);
                //usleep(2000000);
                /* send all parking info */
                memcpy(wbuf,"data",4); // pkg head
                ftime(&tp);
                memcpy(&wbuf[4],(void *)&tp,8);
                swap(8,&wbuf[4]);
                *(int*)&wbuf[12] = get_depot_id();
                swap(4,&wbuf[12]);
                len = 16 + get_all_parking_state(&wbuf[16]);
                while((tcp_send_to_server(len,wbuf)) < len)
                {
                     printf("[SERVER]send to server err\r\n");
                     usleep(1000000);
                }
                need_to_send_to_sever = 0;
            }
        }
        else
        {
            usleep(1000000);
            continue;
        }
        memset(rbuf,0,255);
        len = tcp_listen(rbuf,4);
        if(len <= 0)
        {
            usleep(2000);
            continue;
        }
        if(memcmp("TALL",rbuf,4) == 0)
        {
            printf("[SERVER]TALL cmd\r\n");
            len = tcp_listen(&rbuf[4],16 + get_depot_size() - 4);
            for(loop = 0;loop < get_depot_size();loop ++)
            {
                set_parking_state(loop + 1,rbuf[16 + loop]);
            }
        }
        else if(memcmp("SIZE",rbuf,4) == 0)
        {
            printf("[SERVER]SIZE cmd\r\n");
            len = tcp_listen(&rbuf[4],15 - 4);
            swap(4,&rbuf[4]);
            swap(4,&rbuf[8]);
            swap(4,&rbuf[14]);
            set_depot_info(*(int *)(&rbuf[4]),*(int *)(&rbuf[8]),rbuf[12],*(int *)(&rbuf[14]));
        }
        else if(memcmp("DOWN",rbuf,4) == 0)
        {
            printf("[SERVER]DOWN cmd\r\n");
            len = tcp_listen(&rbuf[4],8 + get_depot_size() * 10 - 4);
            for(loop = 0;loop < get_depot_size();loop ++)
            {
                swap(2,&rbuf[8 + loop * 2 + loop * 8]);
                parking_id_macaddr_mapping(*(unsigned short *)&rbuf[8 + loop * 2 + loop * 8],
                                           &rbuf[8 + 2 + loop * 2 + loop * 8]);

            }

        }
        else if(memcmp("TIME",rbuf,4) == 0)
        {
            printf("[SERVER]TIME cmd\r\n");
            len = tcp_listen(&rbuf[4],8);
            swap(2,&rbuf[8]);
            swap(2,&rbuf[10]);
            freetime = *(unsigned short*)&rbuf[8];
            leavetime = *(unsigned short*)&rbuf[10];
            printf("[SERVER]free time is %d minute\r\n",freetime);

        }
        else if(memcmp("HEAR",rbuf,4) == 0)
        {
            len = tcp_listen(&rbuf[4],8);
        }
        else
        {
            printf("[SERVER]unknown cmd\r\n");
        }
    }
}
