#include "listener.h"
#include <unistd.h>
#include "server_duty.h"
#include <string.h>
#include <stdio.h>
#include <sys/timeb.h>

static const unsigned char mac_addr[8] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
static struct{
    int depot_id;
    int depot_size;
    unsigned char wireless_channel;
    unsigned short net_id;
}depot_info;

typedef struct {
    unsigned short parking_id;
    unsigned char parking_mac_addr[8];
    unsigned char state;
}st_parkingState,*pst_parkingState;

static void swap(unsigned char len,unsigned char *array)
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
    pst_parkingState pstParkingState = NULL;
    tcp_init();
cfg:
    memcpy(wbuf,"size",4); // pkg head
    memcpy((unsigned char *)&wbuf[4],(unsigned char *)mac_addr,8); // mac addr
    while((len = tcp_send_to_server(12,wbuf)) < 12)
    {
        printf("[SERVER]send to server err\r\n");
        usleep(1000000);
    }
    
    len = tcp_listen(rbuf,sizeof(rbuf));
    if(len != 15)
    {
        printf("[SERVER]read again...\r\n");
        goto cfg;
    }
    if(!memcmp(rbuf,"SIZE",4))
    {
        printf("[SERVER]read again...\r\n");
        goto cfg;
    }
    swap(4,&rbuf[4]);
    depot_info.depot_id = *(int *)(&rbuf[4]);
    swap(4,&rbuf[8]);
    depot_info.depot_size = *(int *)(&rbuf[8]);
    depot_info.wireless_channel = rbuf[12];
    swap(4,&rbuf[14]);
    depot_info.net_id = *(int *)(&rbuf[14]);
    
    while((pstParkingState = malloc(sizeof(st_parkingState) * depot_info.depot_size)) == NULL)
    {
        printf("[SERVER]malloc park_info memory failed!\r\n");
    }
down:
    /* download the parking info of this depot */
    len = tcp_listen(rbuf,sizeof(rbuf));
    if(len < (8 + depot_info.depot_size * 10))
    {
        printf("[SERVER]download parking info err\r\n");
        usleep(1000000);
        goto down;
    }
    if(memcmp("DOWN",rbuf,4) != 0)
    {
        printf("[SERVER]is not download parking info cmd\r\n");
        usleep(1000000);
        goto down;
    }
    for(loop = 0;loop <  depot_info.depot_size;loop ++)
    {
        swap(2,&rbuf[8 + loop * 2 + loop * 8]);
        pstParkingState[loop].parking_id = *(int *)&rbuf[8 + loop * 2 + loop * 8];
        swap(8,&rbuf[8 + 2 + loop * 2 + loop * 8]);
        memcpy(pstParkingState[loop].parking_mac_addr,&rbuf[8 + 2 + loop * 2 + loop * 8],8);
        pstParkingState[loop].state = 0;
    }
    
    /* send all parking info */
    memcpy(wbuf,"data",4); // pkg head
    ftime(&tp);
    memcpy(&wbuf[4],(void *)&tp,8);
    swap(8,&wbuf[4]);
    memcpy(&wbuf[12],(void*)&depot_info.depot_id,4);
    swap(4,&wbuf[12]);
    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        wbuf[16 + loop] = pstParkingState[loop].state;
    }
    while((len = tcp_send_to_server(16 + depot_info.depot_size,wbuf)) < 16 + depot_info.depot_size)
    {
        printf("[SERVER]send to server err\r\n");
        usleep(1000000);
    }

    while(1)
    {
        len = tcp_listen(rbuf,sizeof(rbuf));
        if(len < 16 + depot_info.depot_size)
        {
            printf("[SERVER]len err\r\n");
        }
        else if(!memcmp("TALL",rbuf,4))
        {
            printf("[SERVER]cmd err\r\n");
        }
        
        usleep(500000);
    }
}
