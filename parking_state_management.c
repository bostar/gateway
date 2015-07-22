#include <stdio.h>
#include <string.h>
#include "server_duty.h"
#include "parking_state_management.h"

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

typedef enum{
    enOnline = 1,
    enOffline = 0
}en_netState;

static struct{
    int depot_id;
    int depot_size;
    unsigned char wireless_channel;
    unsigned short net_id;
}depot_info;

pst_parkingState pstParkingState = NULL;

void set_depot_info(int depot_id,int depot_size,unsigned char wireless_channel,unsigned short net_id)
{
    depot_info.depot_id = depot_id;
    depot_info.depot_size = depot_size;
    depot_info.wireless_channel = wireless_channel;
    depot_info.net_id = net_id;
    printf("depot_id = %d;depot_size = %d;wireless_channel = %d",depot_info.depot_id,depot_info.depot_size,depot_info.wireless_channel);
}

int get_depot_size(void)
{
    return depot_info.depot_size;
}

int get_depot_id(void)
{
    return depot_info.depot_id;
}

void parking_init(void)
{
    if(pstParkingState == NULL)
    {
        return;
    }
    int loop = 0;
    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        pstParkingState->parking_id = loop;
        memset(pstParkingState->parking_mac_addr,0,8);
        pstParkingState->state = parking_state_idle;
        pstParkingState->online = enOffline;
    }
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
        if(p->state == parking_state_idle)
        {
            p->state = parking_state_prestop;
        }
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
    if(pstParkingState == NULL)
    {
        return 0;
    }

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
    if(pstParkingState == NULL)
    {
        return;
    }

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

void parking_id_macaddr_mapping(unsigned short parking_id,unsigned char *macaddr)
{
    int loop = 0;
    if(pstParkingState == NULL)
    {
        return;
    }

    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        if(pstParkingState->parking_id == parking_id)
        {
            memcpy(pstParkingState->parking_mac_addr,macaddr,8);
            printf("parking_id = %d;parking_mac_addr = 0x%08x%08x\r\n",pstParkingState[loop].parking_id,*(unsigned int*)&pstParkingState[loop].parking_mac_addr[4],*(unsigned int*)&pstParkingState[loop].parking_mac_addr[0]);
            return;
        }
    }
}

int get_all_parking_state(unsigned char* buf)
{
    int loop;
    if(pstParkingState == NULL)
    {
        return 0;
    }
    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        buf[loop] = pstParkingState->state;
    }
    return loop;
}

int set_parking_state(unsigned short parking_id,unsigned char state)
{
    pst_parkingState p;
    p = search_use_netaddr(parking_id);
    p->state = state;
    return 1;
}
