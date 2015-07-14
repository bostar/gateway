#include "listener.h"
#include <unistd.h>
#include "server_duty.h"
#include <string.h>
#include <stdio.h>
#include <sys/timeb.h>
#include "zlg_protocol.h"
static const unsigned char mac_addr[8] = {0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8};
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
    unsigned char online;
}st_parkingState,*pst_parkingState;
pst_parkingState pstParkingState = NULL;

int networking_over(void)
{
    int loop;
    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        if(pstParkingState[loop].online == 1)
        {
            
        }
        else
        {
            return 0;
        }
    }
    return 1;
}

void set_node_online(unsigned char *macaddr)
{
    int loop;
    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        if(memcmp(pstParkingState[loop].parking_mac_addr,macaddr,8) == 0)
        {
            break;
        }
    }
    if(loop < depot_info.depot_size)
    {
        pstParkingState[loop].online = 1;
    }
}

int get_local_addr(unsigned char *local_addr,unsigned char* long_addr)
{
    if(pstParkingState == NULL)
    {
        return -1;
    }
    else
    {
        local_addr[0] = 0x00;
        local_addr[1] = 0x01;
        return 0;
    }
}
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
    depot_info.depot_id = *(int *)(&rbuf[4]);
    swap(4,&rbuf[8]);
    depot_info.depot_size = *(int *)(&rbuf[8]);
    depot_info.wireless_channel = rbuf[12];
    swap(4,&rbuf[14]);
    depot_info.net_id = *(int *)(&rbuf[14]);
    printf("depot_id = %d;depot_size = %d;wireless_channel = %d",depot_info.depot_id,depot_info.depot_size,depot_info.wireless_channel);
    while((pstParkingState = malloc(sizeof(st_parkingState) * depot_info.depot_size)) == NULL)
    {
        printf("[SERVER]malloc park_info memory failed!\r\n");
    }
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
    for(loop = 0;loop <  depot_info.depot_size;loop ++)
    {
        swap(2,&rbuf[8 + loop * 2 + loop * 8]);
        pstParkingState[loop].parking_id = *(int *)&rbuf[8 + loop * 2 + loop * 8];
        swap(8,&rbuf[8 + 2 + loop * 2 + loop * 8]);
        memcpy(pstParkingState[loop].parking_mac_addr,&rbuf[8 + 2 + loop * 2 + loop * 8],8);
        pstParkingState[loop].state = 0;
        pstParkingState[loop].online = 0;
        printf("parking_id = %d;parking_mac_addr = 0x%08x%08x\r\n",pstParkingState[loop].parking_id,*(unsigned int*)&pstParkingState[loop].parking_mac_addr[4],*(unsigned int*)&pstParkingState[loop].parking_mac_addr[0]);
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
        printf("parking%d state is 0x%02x\r\n",loop + 1,pstParkingState[loop].state);
    }
    while((len = tcp_send_to_server(16 + depot_info.depot_size,wbuf)) < 16 + depot_info.depot_size)
    {
        printf("[SERVER]send to server err\r\n");
        usleep(1000000);
    }

    while(1)
    {
        usleep(1000000);
        len = tcp_listen(rbuf,sizeof(rbuf));
        if(memcmp("TALL",rbuf,4) != 0)
        {
            printf("[SERVER]cmd err\r\n");
        }
        for(loop = 0;loop < depot_info.depot_size;loop ++)
        {
            if(rbuf[16 + loop] != pstParkingState[loop].state)
            {
                pstParkingState[loop].state = rbuf[16 + loop];
                if(pstParkingState[loop].state == 0x81)
                {
                    switchLockControl((unsigned short)(loop + 1),0x00);
                }
                else
                {
                    switchLockControl((unsigned short)(loop + 1),0x01);
                }
            }
            printf("parking%d state is 0x%02x\r\n",loop + 1,rbuf[16 + loop]);
        }
        
        
        usleep(1000000);
    }
}
