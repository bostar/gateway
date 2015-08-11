#include "listener.h"
#include <unistd.h>
#include "server_duty.h"
#include <string.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <pthread.h>
#include "zlg_protocol.h"
#include "parking_state_management.h"
static const unsigned char mac_addr[8] = {0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8};

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
    static unsigned char wbuf[255],rbuf[255];
    int len;
    int loop = 0;
    struct timeb tp;
    int ret;
    pthread_t id;
    //static unsigned char ctl;
    tcp_init();
cfg:
    memcpy(wbuf,"size",4); // pkg head
    memcpy((unsigned char *)&wbuf[4],(unsigned char *)mac_addr,8); // mac addr
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
    len = tcp_listen(rbuf,sizeof(rbuf));
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
        if(need_to_send_to_sever == 1)
        {
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
        len = tcp_listen(rbuf,sizeof(rbuf));
        if(len <= 0)
        {
            continue;
        }
        if(memcmp("TALL",rbuf,4) != 0)
        {
            printf("[SERVER]cmd err\r\n");
            continue;
        }
        for(loop = 0;loop < get_depot_size();loop ++)
        {
            set_parking_state(loop + 1,rbuf[16 + loop]);
        }
    }
}
