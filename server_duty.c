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

typedef enum{
    parking_state_idle = 0x00, // 空闲
    parking_state_prestop = 0x01, // 车来
    parking_state_stop = 0x03, // 车来超N分钟已上锁
    parking_state_stop_err = 0x04, // 车来超N分钟但加锁失败（硬件故障）
    parking_state_booking = 0xfe, // 内部使用
    parking_state_booking_busy = 0x1a, // 预定车位失败（被抢占）
    parking_state_booking_err = 0x1B, // 预定车位，上锁失败（硬件故障）
    parking_state_have_booked = 0x09, // 预定成功，且车位已上锁
    parking_state_have_booked_err = 0x1b, // 预定车位，上锁失败（硬件故障）
    parking_state_have_paid = 0x05, // 支付后解锁成功
    parking_state_have_paid_err = 0x08 // 支付后解锁硬件异常
}en_parking_state;

typedef struct {
    unsigned short parking_id;
    unsigned char parking_mac_addr[8];
    unsigned char state;
    unsigned char online;
}st_parkingState,*pst_parkingState;
pst_parkingState pstParkingState = NULL;

pst_parkingState search_use_netaddr(unsigned short netaddr)
{
    int loop;
    if(pstParkingState == NULL)
    {
        return NULL;
    }
    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        if(netaddr == pstParkingState[loop].parking_id)
        {
            return &pstParkingState[loop];
        }
    }
    return NULL;
}

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

void event_report(unsigned short netaddr,unsigned char event)
{
    pst_parkingState p;
    p = search_use_netaddr(netaddr);
    if(p == NULL)
    {
        return ;
    }
    printf("[SERVER]:event report-netaddr = %04x ,event = %02x;",netaddr,event);
    switch(event)
    {
        case en_vehicle_comming:
        break;
        case en_vehicle_leave:
        break;
        case en_lock_success:
        break;
        case en_lock_failed:
        break;
        case en_unlock_success:
        break;
        case en_unlock_failed:
        break;
        default:
        break;
    }
}

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
    int loop = 0;
    //for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        /*if(memcmp(pstParkingState[loop].parking_mac_addr,macaddr,8) == 0)
        {
            break;
        }*/
    }
    //if(loop < depot_info.depot_size)
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
        *(unsigned short*)local_addr = 0x0001;
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
        pstParkingState[loop].state = parking_state_idle;
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
        /*printf("before run \r\n");
        if(networking_over() != 1)
        {
            continue;
        }
        printf("run \r\n");
        if(ctl)
        {
            ctl = 0;
            switchLockControl(0x0001,0x00);
        }
        else
        {
            ctl = 1;
            switchLockControl(0x0001,0x03);

        }*/
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
