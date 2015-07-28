#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "server_duty.h"
#include "parking_state_management.h"
#include "ctl_cmd_cache.h"

typedef enum{
    parking_state_idle = 0x00, // 空闲
    parking_state_prestop = 0x01, // 车来
    parking_state_stop_lock = 0x03, // 车来超N分钟已上锁
    parking_state_stop_lock_failed = 0x04, // 车来超N分钟但加锁失败（硬件故障）
    parking_state_booking = 0x81, // 车位被预定
    parking_state_booking_lock = 0x09, // 预定成功，且车位已上锁
    parking_state_booking_busy = 0x1a, // 预定车位失败（被抢占）
    parking_state_booking_lock_failed = 0x1b, // 预定车位，上锁失败（硬件故障）
    //parking_state_booked_coming = 0x82, // 被预定车位解锁，车主到达现场
    parking_state_booked_coming_unlock = 0x1c, // 被预定车位解锁成功
    parking_state_booked_coming_unlock_failed = 0x1d, // 被预定车位解锁失败
    parking_state_booked_coming_lock = 0xf1, // 被预定车位，车到达，已上锁
    parking_state_booked_coming_lock_failed = 0xf2, // 被预定车位，车到达，上锁失败
    //parking_state_unbooking = 0x83, // 取消预定
    parking_state_unbooking_unlock = 0x1e, // 取消预定成功已解锁
    parking_state_unbooking_unlock_failed = 0x1f, // 取消预定失败，硬件故障
    //parking_state_have_paid = 0x84, // 已支付
    parking_state_have_paid_unlock = 0x05, // 支付后解锁成功
    parking_state_have_paid_unlock_failed = 0x08, // 支付后解锁硬件异常
    en_parking_state_max = 0xff
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
pthread_mutex_t parking_info_mutex = PTHREAD_MUTEX_INITIALIZER;
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
    int loop;
    pthread_mutex_init(&parking_info_mutex,NULL);
    pthread_mutex_lock(&parking_info_mutex);
    if(pstParkingState == NULL)
    {
        pthread_mutex_unlock(&parking_info_mutex);
        return;
    }
    
    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        pstParkingState->parking_id = loop + 1;
        memset(pstParkingState->parking_mac_addr,0,8);
        pstParkingState->state = parking_state_idle;
        pstParkingState->online = enOffline;
    }
    pthread_mutex_unlock(&parking_info_mutex);
}

char* const parking_state_string[en_parking_state_max] = {
    [parking_state_idle] = "parking_state_idle", // 空闲
    [parking_state_prestop] = "parking_state_prestop", // 车来
    [parking_state_stop_lock] = "parking_state_stop_lock", // 车来超N分钟已上锁
    [parking_state_stop_lock_failed] = "parking_state_stop_lock_failed", // 车来超N分钟但加锁失败（硬件故障）
    [parking_state_booking] = "parking_state_booking", // 车位被预定
    [parking_state_booking_lock] = "parking_state_booking_lock", // 预定成功，且车位已上锁
    [parking_state_booking_busy] = "parking_state_booking_busy", // 预定车位失败（被抢占）
    [parking_state_booking_lock_failed] = "parking_state_booking_lock_failed", // 预定车位，上锁失败（硬件故障）
    //[parking_state_booked_coming] = "parking_state_booked_coming", // 被预定车位解锁，车主到达现场
    [parking_state_booked_coming_unlock] = "parking_state_booked_coming_unlock", // 被预定车位解锁成功
    [parking_state_booked_coming_unlock_failed] = "parking_state_booked_coming_unlock_failed", // 被预定车位解锁失败
    [parking_state_booked_coming_lock] = "parking_state_booked_coming_lock", // 被预定车位，车到达，已上锁
    [parking_state_booked_coming_lock_failed] = "parking_state_booked_coming_lock_failed", // 被预定车位，车到达，上锁>失败
    //[parking_state_unbooking] = "parking_state_unbooking", // 取消预定
    [parking_state_unbooking_unlock] = "parking_state_unbooking_unlock", // 取消预定成功已解锁
    [parking_state_unbooking_unlock_failed] = "parking_state_unbooking_unlock_failed", // 取消预定失败，硬件故障
    //[parking_state_have_paid] = "parking_state_have_paid", // 已支付
    [parking_state_have_paid_unlock] = "parking_state_have_paid_unlock", // 支付后解锁成功
    [parking_state_have_paid_unlock_failed] = "parking_state_have_paid_unlock_failed", // 支付后解锁硬件异常
};

void parking_state_check_routin(void)
{
    int loop;
    time_t time_in_second;
    while(1)
    {
    usleep(1000000);
    time_in_second = time((time_t*)NULL);
    pthread_mutex_lock(&parking_info_mutex);
    if(pstParkingState == NULL)
    {
        pthread_mutex_unlock(&parking_info_mutex);
        return;
    }
    //printf("=========== parking state ==============\r\n");
    for(loop = 0;loop < get_depot_size();loop ++)
    {
       // printf("[SERVER:]%04x ",pstParkingState[loop].parking_id);
       // printf("%s",parking_state_string[pstParkingState[loop].state]);
       // printf("\r\n");
        switch(pstParkingState[loop].state)
        {
            case parking_state_idle: // 空闲
                break;
            case parking_state_prestop: // 车来
                if(time_in_second - pstParkingState[loop].time > 3) // second
                {
                    putCtlCmd(pstParkingState[loop].parking_id,en_order_lock);
                }
                break;
            case parking_state_stop_lock: // 车来超N分钟已上锁
                break;
            case parking_state_stop_lock_failed: // 车来超N分钟但加锁失败（硬件故障）
                if(time_in_second - pstParkingState[loop].time > 3) // second
                {
                    putCtlCmd(pstParkingState[loop].parking_id,en_order_lock);
                }
                break;
            case parking_state_booking: // 车位被预定
                break;
            case parking_state_booking_lock: // 预定成功，且车位已上锁
                break;
            case parking_state_booking_busy: // 预定车位失败（被抢占）
                break;
            case parking_state_booking_lock_failed: // 预定车位，上锁失败（硬件故>障）
                if(time_in_second - pstParkingState[loop].time > 3) // second
                {
                    putCtlCmd(pstParkingState[loop].parking_id,en_order_lock);
                }
                break;
            /*case parking_state_booked_coming: // 被预定车位解锁，车主到达现场
                if(time_in_second - pstParkingState[loop].time > 3) // second
                {
                    putCtlCmd(pstParkingState[loop].parking_id,en_order_unlock);
                }
                break;*/
            case parking_state_booked_coming_unlock: // 被预定车位解锁成功
                if(time_in_second - pstParkingState[loop].time > 3) // second
                {
                    pstParkingState[loop].state = parking_state_idle;
                }
                pstParkingState[loop].time = time((time_t*)NULL);
                pstParkingState[loop].state = parking_state_prestop;
                break;
            case parking_state_booked_coming_unlock_failed: // 被预定车位解锁失败
                if(time_in_second - pstParkingState[loop].time > 3) // second
                {
                    putCtlCmd(pstParkingState[loop].parking_id,en_order_unlock);
                }
                break;
            case parking_state_booked_coming_lock: // 被预定车位，车到达，已上锁
                break;
            case parking_state_booked_coming_lock_failed: // 被预定车位，车到达，上锁失败
                if(time_in_second - pstParkingState[loop].time > 3) // second
                {
                    putCtlCmd(pstParkingState[loop].parking_id,en_order_lock);
                }
                break;
            /*case parking_state_unbooking: // 取消预定
                break;*/
            case parking_state_unbooking_unlock: // 取消预定成功已解锁
                break;
            case parking_state_unbooking_unlock_failed: // 取消预定失败，硬件故障
                if(time_in_second - pstParkingState[loop].time > 3) // second
                {
                    putCtlCmd(pstParkingState[loop].parking_id,en_order_unlock);
                }
                break;
            /*case parking_state_have_paid: // 已支付
                break;*/
            case parking_state_have_paid_unlock: // 支付后解锁成功
                break;
            case parking_state_have_paid_unlock_failed: // 支付后解锁硬件异常
                if(time_in_second - pstParkingState[loop].time > 3) // second
                {
                    putCtlCmd(pstParkingState[loop].parking_id,en_order_unlock);
                }
                break;
            default:
                break;
        }
    }
    pthread_mutex_unlock(&parking_info_mutex);
    }
}

char * const event_string[en_max_event] = {"en_vehicle_comming\r\n","en_vehicle_leave\r\n","en_lock_success\r\n","en_lock_failed\r\n","en_unlock_success\r\n","en_unlock_failed\r\n"};

void event_report(unsigned short netaddr,unsigned char event)
{
    pst_parkingState p;
    time_t time_in_second = time((time_t *)NULL);
    printf("[SERVER:]%04x ",netaddr);
    printf("%s",event_string[event]);
    pthread_mutex_lock(&parking_info_mutex);
    p = search_use_netaddr(netaddr);
    if(p == NULL)
    {
        pthread_mutex_unlock(&parking_info_mutex);
        return ;
    }
    printf("[SERVER]:event report-netaddr = %04x ,event = %02x;",netaddr,event);
    switch(event)
    {
        case en_vehicle_comming:
        if(p->state == parking_state_idle)
        {
            p->state = parking_state_prestop;
            p->time = time_in_second; // second
        }
        else if(p->state == parking_state_booked_coming_unlock)
        {
            p->time = time_in_second;
        }
        else
        {
            
        }
        break;
        case en_vehicle_leave:
        if(p->state == parking_state_prestop)
        {
            p->state = parking_state_idle;
        }
        if(p->state == parking_state_booked_coming_unlock)
        {
            p->state = parking_state_idle;
        }
        if(p->state == parking_state_have_paid_unlock)
        {
            p->state = parking_state_idle;
        }
        
        break;
        case en_lock_success:
        if(p->state == parking_state_prestop)
        {
            p->state = parking_state_stop_lock;
        }
        if(p->state == parking_state_booking)
        {
            p->state = parking_state_booking_lock;
        }
        if(p->state == parking_state_booked_coming_unlock)
        {
            p->state = parking_state_booked_coming_lock;
        }
        break;
        case en_lock_failed:
        p->time =  time_in_second;
        if(p->state == parking_state_prestop)
        {
            p->state = parking_state_stop_lock_failed;
        }
        if(p->state == parking_state_booking)
        {
            p->state = parking_state_booking_lock_failed;
        }
        if(p->state == parking_state_booked_coming_unlock)
        {
            p->state = parking_state_booked_coming_lock_failed;
        }
        break;
        case en_unlock_success:
        /*if(p->state == parking_state_booked_coming)
        {
            p->state = parking_state_booked_coming_unlock;
        }*/
        /*if(p->state == parking_state_have_paid)
        {
            p->state = parking_state_have_paid_unlock;
        }*/
        /*if(p->state == parking_state_unbooking)
        {
            p->state = parking_state_idle;
        }*/
        break;
        case en_unlock_failed:
        p->time = time_in_second;
        /*if(p->state == parking_state_booked_coming)
        {
            p->state = parking_state_booked_coming_unlock_failed;
        }*/
        /*if(p->state == parking_state_have_paid)
        {
            p->state = parking_state_have_paid_unlock_failed;
        }
        if(p->state == parking_state_unbooking)
        {
            p->state = parking_state_unbooking_unlock_failed;
        }*/
        break;
        default:
        break;
    }
    pthread_mutex_unlock(&parking_info_mutex);
}

int networking_over(void)
{
    int loop;
    pthread_mutex_lock(&parking_info_mutex);
    if(pstParkingState == NULL)
    {
        pthread_mutex_unlock(&parking_info_mutex);
        return 0;
    }

    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        if(pstParkingState[loop].online == 1)
        {

        }
        else
        {
            pthread_mutex_unlock(&parking_info_mutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&parking_info_mutex);
    return 1;
}

void set_node_online(unsigned char *macaddr)
{
    int loop = 0;
    pthread_mutex_lock(&parking_info_mutex);
    if(pstParkingState == NULL)
    {
        pthread_mutex_unlock(&parking_info_mutex);
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
    pthread_mutex_unlock(&parking_info_mutex);
}

int get_local_addr(unsigned char *local_addr,unsigned char* long_addr)
{
    pthread_mutex_lock(&parking_info_mutex);
    if(pstParkingState == NULL)
    {
        pthread_mutex_unlock(&parking_info_mutex);
        return -1;
    }
    else
    {
        *(unsigned short*)local_addr = 0x0001;
        pthread_mutex_unlock(&parking_info_mutex);
        return 0;
    }
}

pst_parkingState search_use_netaddr(unsigned short netaddr)
{
    int loop;
    //pthread_mutex_lock(&parking_info_mutex);
    if(pstParkingState == NULL)
    {
        //pthread_mutex_unlock(&parking_info_mutex);
        return NULL;
    }
    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        if(netaddr == pstParkingState[loop].parking_id)
        {
            //pthread_mutex_unlock(&parking_info_mutex);
            return &pstParkingState[loop];
        }
    }
    //pthread_mutex_lock(&parking_info_mutex);
    return NULL;
}

void parking_id_macaddr_mapping(unsigned short parking_id,unsigned char *macaddr)
{
    int loop = 0;
    pthread_mutex_lock(&parking_info_mutex);
    if(pstParkingState == NULL)
    {
        pthread_mutex_unlock(&parking_info_mutex);
        return;
    }

    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        if(pstParkingState->parking_id == parking_id)
        {
            memcpy(pstParkingState->parking_mac_addr,macaddr,8);
            printf("parking_id = %d;parking_mac_addr = 0x%08x%08x\r\n",pstParkingState[loop].parking_id,*(unsigned int*)&pstParkingState[loop].parking_mac_addr[4],*(unsigned int*)&pstParkingState[loop].parking_mac_addr[0]);
            pthread_mutex_unlock(&parking_info_mutex);
            return;
        }
    }
    pthread_mutex_unlock(&parking_info_mutex);
}

int get_all_parking_state(unsigned char* buf)
{
    int loop;
    pthread_mutex_lock(&parking_info_mutex);
    if(pstParkingState == NULL)
    {
        pthread_mutex_unlock(&parking_info_mutex);
        return 0;
    }
    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        buf[loop] = pstParkingState->state;
    }
    pthread_mutex_unlock(&parking_info_mutex);
    return loop;
}

int set_parking_state(unsigned short parking_id,unsigned char state)
{
    pst_parkingState p;
    pthread_mutex_lock(&parking_info_mutex);
    p = search_use_netaddr(parking_id);
    if((p == NULL) || (p->state == state))
    {
        pthread_mutex_unlock(&parking_info_mutex);
        return 1;
    }
printf("0x%04x,state is 0x%02x",parking_id,state); 
    switch(state)
    {
        case parking_state_booking:
            //if(p->state == parking_state_idle)
            {
                p->state = parking_state_booking;
                putCtlCmd(parking_id,en_order_lock);
            }
            //else
            {
              //  p->state = parking_state_booking_busy;
            }
            break;
        case parking_state_idle:
            //if(p->state == parking_state_booking_lock)
            {
                p->state = parking_state_idle;
                p->time = time((time_t)NULL);
                putCtlCmd(parking_id,en_order_unlock);
            }
            //else
            {
                //p->state = parking_state_booking_busy;
            }
            break;
        /*case parking_state_unbooking:
            if((p->state == parking_state_booking) 
               || (p->state == parking_state_booking_lock)
               || (p->state == parking_state_booking_busy) 
               || (p->state == parking_state_booking_lock_failed)
               || (p->state == parking_state_booked_coming_unlock)
               || (p->state == parking_state_booked_coming_unlock_failed)
               || (p->state == parking_state_booked_coming_lock)
               || (p->state == parking_state_booked_coming_lock_failed))
            {
                p->state = parking_state_unbooking;
                putCtlCmd(parking_id,en_order_unlock);
            }
            else
            {
                
            }
            break;
        case parking_state_have_paid:
            switch(p->state)
            {
                case parking_state_stop_lock:
                    putCtlCmd(parking_id,en_order_unlock);
                    break;
                case parking_state_booked_coming_lock:
                    putCtlCmd(parking_id,en_order_unlock);
                    break;
                default:
                    break;
            }*/
        default:
            break;
    }
    
    pthread_mutex_unlock(&parking_info_mutex);
    return 1;
}
