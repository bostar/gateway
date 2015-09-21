#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "server_duty.h"
#include "parking_state_management.h"
#include "xbee_protocol.h"
#include <sys/timeb.h>
#include "listener.h"

extern void swap(unsigned char len,unsigned char *array);
void set_online(unsigned short netaddr);
typedef enum{
    parking_state_idle = 0x00, // 空闲
    parking_state_prestop = 0x01, // 车来
    parking_state_stop_lock = 0x03, // 车来超N分钟已上锁
    parking_state_stop_lock_failed = 0x04, // 车来超N分钟但加锁失败（硬件故障）
    parking_state_booking = 0x81, // 车位被预定
    parking_state_booking_lock = 0x09, // 预定成功，且车位已上锁
    parking_state_booking_busy = 0x1a, // 预定车位失败（被抢占）
    parking_state_booking_lock_failed = 0x1b, // 预定车位，上锁失败（硬件故障）
    parking_state_booked_coming = 0x82, // 被预定车位解锁，车主到达现场
    parking_state_booked_coming_unlock = 0x1c, // 被预定车位解锁成功
    parking_state_booked_coming_unlock_failed = 0x1d, // 被预定车位解锁失败
    parking_state_booked_coming_unlock_goto_idle = 0x20,// 降锁，免费时间内车未到达（即将转为空闲）
    parking_state_booked_coming_lock = 0x21, // 被预定车位，车到达，已上锁
    parking_state_booked_coming_lock_failed = 0x22, // 被预定车位，车到达，上锁失败
    parking_state_unbooking = 0x83, // 取消预定
    parking_state_unbooking_unlock = 0x1e, // 取消预定成功已解锁
    parking_state_unbooking_unlock_failed = 0x1f, // 取消预定失败，硬件故障
    parking_state_have_paid = 0x84, // 已支付
    parking_state_have_paid_unlock = 0x05, // 支付后解锁成功
    parking_state_have_paid_unlock_vehicle_leave = 0x06, // 支付解锁成功N分钟内车离开
    parking_state_have_paid_relock = 0x23, // 支付解锁后车未离开重新加锁计费
    parking_state_have_paid_unlock_failed = 0x08, // 支付后解锁硬件异常
    parking_state_have_paid_relock_failed = 0x24, // 支付解锁后车未离开重新加锁失败
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
unsigned char need_to_send_to_sever = 1;

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
    //printf("depot_size is %d\r\n",depot_info.depot_size);
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
        pstParkingState[loop].parking_id = loop + 1;
        memset(pstParkingState[loop].parking_mac_addr,0,8);
        pstParkingState[loop].netaddr = 0;
        pstParkingState[loop].state = parking_state_idle;
        need_to_send_to_sever = 1;
        pstParkingState[loop].online = enOffline;
        pstParkingState[loop].event = en_max_event;
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
    [parking_state_booked_coming] = "parking_state_booked_coming", // 被预定车位解锁，车主到达现场
    [parking_state_booked_coming_unlock] = "parking_state_booked_coming_unlock", // 被预定车位解锁成功
    [parking_state_booked_coming_unlock_failed] = "parking_state_booked_coming_unlock_failed", // 被预定车位解锁失败
    [parking_state_booked_coming_unlock_goto_idle] = "parking_state_booked_coming_unlock_goto_idle", // 降锁，免费时间内车未到达（即将转为空闲）
    [parking_state_booked_coming_lock] = "parking_state_booked_coming_lock", // 被预定车位，车到达，已上锁
    [parking_state_booked_coming_lock_failed] = "parking_state_booked_coming_lock_failed", // 被预定车位，车到达，上锁>失败
    [parking_state_unbooking] = "parking_state_unbooking", // 取消预定
    [parking_state_unbooking_unlock] = "parking_state_unbooking_unlock", // 取消预定成功已解锁
    [parking_state_unbooking_unlock_failed] = "parking_state_unbooking_unlock_failed", // 取消预定失败，硬件故障
    [parking_state_have_paid] = "parking_state_have_paid", // 已支付
    [parking_state_have_paid_unlock] = "parking_state_have_paid_unlock", // 支付后解锁成功
    [parking_state_have_paid_unlock_vehicle_leave] = "parking_state_have_paid_unlock_vehicle_leave", // 支付解锁成功N分钟内车离开
    [parking_state_have_paid_unlock_failed] = "parking_state_have_paid_unlock_failed", // 支付后解锁硬件异常
    [parking_state_have_paid_relock] = "parking_state_have_paid_relock", // 支付解锁后车未离开重新加锁计费
    [parking_state_have_paid_relock_failed] = "parking_state_have_paid_relock_failed", // 支付解锁后车未离开重新加锁失败
};

char* const parking_online_string[2] = {
    [enOnline] = "online",
    [enOffline] = "offline",
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
        continue;
    }
    printf("=========== parking state ==============\r\n");
    for(loop = 0;loop < get_depot_size();loop ++)
    {
        printf("[SERVER:]%04x ",pstParkingState[loop].parking_id);
        printf("%s; ",parking_online_string[pstParkingState[loop].online]);
        printf("[SERVER:]%04x ",pstParkingState[loop].netaddr);
        printf("%s",parking_state_string[pstParkingState[loop].state]);
        printf("\r\n");
        if(pstParkingState[loop].online == 1)
        {
            if((time_in_second - pstParkingState[loop].offline_time_out) > 12)
            {
                pstParkingState[loop].online = 0;
            }
        }
        switch(pstParkingState[loop].state)
        {
            case parking_state_idle: // 空闲
                break;
            case parking_state_prestop: // 车来
                if(time_in_second - pstParkingState[loop].time > 15)//freetime * 60) // second
                {
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_lock);
                    pstParkingState[loop].time = time((time_t*)NULL);
                }
                break;
            case parking_state_stop_lock: // 车来超N分钟已上锁
                break;
            case parking_state_stop_lock_failed: // 车来超N分钟但加锁失败（硬件故障）
                if(time_in_second - pstParkingState[loop].time > 5) // second
                {
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_lock);
                    pstParkingState[loop].time = time((time_t*)NULL);
                }
                break;
            case parking_state_booking: // 车位被预定
                if(time_in_second - pstParkingState[loop].time > 5) // secon    d
                {
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_lock);
                    pstParkingState[loop].time = time((time_t*)NULL);
                }

                break;
            case parking_state_booking_lock: // 预定成功，且车位已上锁
                break;
            case parking_state_booking_busy: // 预定车位失败（被抢占）
                break;
            case parking_state_booking_lock_failed: // 预定车位，上锁失败（硬件故>障）
                if(time_in_second - pstParkingState[loop].time > 5) // second
                {
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_lock);
                    pstParkingState[loop].time = time((time_t*)NULL);
                }
                break;
            case parking_state_booked_coming: // 被预定车位解锁，车主到达现场
                if(time_in_second - pstParkingState[loop].time > 5) // second
                {
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_unlock);
                    pstParkingState[loop].time = time((time_t*)NULL);
                }
                break;
            case parking_state_booked_coming_unlock: // 被预定车位解锁成功
                if(time_in_second - pstParkingState[loop].time > 15) // second
                {
                    need_to_send_to_sever = 1;
                    pstParkingState[loop].state = parking_state_booked_coming_unlock_goto_idle;
                    pstParkingState[loop].time = time((time_t*)NULL);
                }
                break;
                //pstParkingState[loop].time = time((time_t*)NULL);
                //pstParkingState[loop].state = parking_state_prestop;

            case parking_state_booked_coming_unlock_goto_idle:
                if(time_in_second - pstParkingState[loop].time > 2) // second
                {
                    need_to_send_to_sever = 1;
                    pstParkingState[loop].state = parking_state_idle;
                }
                break;
            case parking_state_booked_coming_unlock_failed: // 被预定车位解锁失败
                if(time_in_second - pstParkingState[loop].time > 5) // second
                {
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_unlock);
                    pstParkingState[loop].time = time((time_t*)NULL);
                }
                break;
            case parking_state_booked_coming_lock: // 被预定车位，车到达，已上锁
                break;
            case parking_state_booked_coming_lock_failed: // 被预定车位，车到达，上锁失败
                if(time_in_second - pstParkingState[loop].time > 5) // second
                {
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_lock);
                    pstParkingState[loop].time = time((time_t*)NULL);
                }
                break;
            case parking_state_unbooking: // 取消预定
                if(time_in_second - pstParkingState[loop].time > 5) // secon    d
                {
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_unlock);
                    pstParkingState[loop].time = time((time_t*)NULL);
                }
                break;
            case parking_state_unbooking_unlock: // 取消预定成功已解锁
                if(time_in_second - pstParkingState[loop].time > 2) // secon    d
                {
                    need_to_send_to_sever = 1;
                    pstParkingState[loop].state = parking_state_idle;
                }
                break;
            case parking_state_unbooking_unlock_failed: // 取消预定失败，硬件故障
                if(time_in_second - pstParkingState[loop].time > 5) // second
                {
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_unlock);
                    pstParkingState[loop].time = time((time_t*)NULL);
                }
                break;
            case parking_state_have_paid: // 已支付
                if(time_in_second - pstParkingState[loop].time > 5) // secon    d               
                {
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_unlock);
                    pstParkingState[loop].time = time((time_t*)NULL);
                }

                break;
            case parking_state_have_paid_unlock: // 支付后解锁成功
                if(time_in_second - pstParkingState[loop].time > freetime * 60) // secon    d               
                if(time_in_second - pstParkingState[loop].time > 15)//freetime * 60) // secon    d               
                {
                    need_to_send_to_sever = 1;
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_lock);
                    pstParkingState[loop].time = time((time_t*)NULL);
                    pstParkingState[loop].state = parking_state_have_paid_relock;
                }
                break;
            case parking_state_have_paid_unlock_vehicle_leave:
                if(time_in_second - pstParkingState[loop].time > 2) // second
                {
                    need_to_send_to_sever = 1;
                    pstParkingState[loop].state = parking_state_idle;
                }
                break;
            case parking_state_have_paid_relock: // 支付解锁后车未离开重新加锁计费
                if(time_in_second - pstParkingState[loop].time > 5) // secon    d               
                {
                    need_to_send_to_sever = 1;
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_lock);
                    pstParkingState[loop].time = time((time_t*)NULL);
                    pstParkingState[loop].state = parking_state_have_paid_relock_failed;
                }
                break;
            case parking_state_have_paid_relock_failed: // 支付解锁后车未离开重新加锁计费
                if(time_in_second - pstParkingState[loop].time > 5) // secon    d               
                {
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_lock);
                    pstParkingState[loop].time = time((time_t*)NULL);
                }
                break;
            case parking_state_have_paid_unlock_failed: // 支付后解锁硬件异常
                if(time_in_second - pstParkingState[loop].time > 5) // second
                {
                    XBeePutCtlCmd(pstParkingState[loop].parking_mac_addr,pstParkingState[loop].netaddr,en_order_unlock);
                    pstParkingState[loop].time = time((time_t*)NULL);
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
    set_online(netaddr);
    if(p->event == event)
    {
        switch(event)
        {
            case en_lock_success:
                if((p->state == parking_state_idle) || (p->state == parking_state_booked_coming_unlock) || (p->state == parking_state_have_paid_unlock))
                {
                    XBeePutCtlCmd(p->parking_mac_addr,p->netaddr,en_order_unlock);
                }
                break;
            case en_unlock_success:
                if((p->state == parking_state_stop_lock) || (p->state == parking_state_booking_lock) || (p->state == parking_state_booked_coming_lock))
                {
                    XBeePutCtlCmd(p->parking_mac_addr,p->netaddr,en_order_lock);
                }
                break;
            default:
                break;
        }

        pthread_mutex_unlock(&parking_info_mutex);
        return ;
    }
    else
    {
        p->event = event;
    }
    switch(event)
    {
        case en_vehicle_comming:
        if((p->state == parking_state_idle) || (p->state == parking_state_booked_coming_unlock_goto_idle) || (p->state == parking_state_unbooking_unlock) || (p->state == parking_state_have_paid_unlock_vehicle_leave))
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_prestop;
            p->time = time_in_second; // second
        }
        else if(p->state == parking_state_booked_coming_unlock)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_prestop;
            p->time = time_in_second;
        }
        else
        {
            
        }
        break;
        case en_vehicle_leave:
        if(p->state == parking_state_prestop)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_idle;
        }
        /*if(p->state == parking_state_booked_coming_unlock)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_idle;
        }*/
        if(p->state == parking_state_have_paid_relock)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_idle;
            XBeePutCtlCmd(p->parking_mac_addr,p->netaddr,en_order_unlock);
            //p->time = time_in_second; // second
        }
        if(p->state == parking_state_have_paid_relock_failed)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_idle;
            XBeePutCtlCmd(p->parking_mac_addr,p->netaddr,en_order_unlock);
            //p->time = time_in_second; // second
        }
        /*if(p->state == parking_state_have_paid)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_idle;
            p->time = time_in_second; // second
        }*/


        if(p->state == parking_state_have_paid_unlock)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_have_paid_unlock_vehicle_leave;
            p->time = time_in_second; // second
        }
        
        break;
        case en_lock_success:
        if((p->state == parking_state_idle) || (p->state == parking_state_booked_coming_unlock) || (p->state == parking_state_have_paid_unlock))
        {
            XBeePutCtlCmd(p->parking_mac_addr,p->netaddr,en_order_unlock);
        }

        if((p->state == parking_state_prestop))// && (time_in_second - p->time > 15))
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_stop_lock;
        }
        /*else
        {
            putCtlCmd(p->parking_id,en_order_unlock);
        }*/
        if(p->state == parking_state_stop_lock_failed)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_stop_lock;
        }
        if(p->state == parking_state_booking || p->state == parking_state_booking_lock_failed)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_booking_lock;
        }
        if(p->state == parking_state_have_paid_relock)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_stop_lock;
        }
        if(p->state == parking_state_have_paid_relock_failed)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_stop_lock;
        }
        break;
        case en_lock_failed:
        p->time =  time_in_second;
        if(p->state == parking_state_prestop)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_stop_lock_failed;
            p->time = time_in_second; // second
        }
        if(p->state == parking_state_booking)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_booking_lock_failed;
            p->time = time_in_second; // second
        }
        if(p->state == parking_state_have_paid_relock)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_have_paid_relock_failed;
            p->time = time_in_second; // second
        }
        if(p->state == parking_state_have_paid_relock_failed)
        {
            p->time = time_in_second; // second
        }
        break;
        case en_unlock_success:
        if((p->state == parking_state_stop_lock) || (p->state == parking_state_booking_lock) || (p->state == parking_state_booked_coming_lock))
        {
            XBeePutCtlCmd(p->parking_mac_addr,p->netaddr,en_order_lock);
        }
        if(p->state == parking_state_booked_coming || p->state == parking_state_booked_coming_unlock_failed)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_booked_coming_unlock;
            p->time = time_in_second; // second
        }
        if(p->state == parking_state_have_paid || p->state == parking_state_have_paid_unlock_failed)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_have_paid_unlock;
            p->time = time_in_second; // second
            
        }
        if(p->state == parking_state_unbooking || p->state == parking_state_unbooking_unlock_failed)
        {
            need_to_send_to_sever = 1;
            p->state = parking_state_unbooking_unlock;
            p->time = time_in_second; // second
        }
        break;
        case en_unlock_failed:
        p->time = time_in_second;
        if(p->state == parking_state_booked_coming)
        {
            need_to_send_to_sever = 1;
            p->time = time_in_second; // second
            p->state = parking_state_booked_coming_unlock_failed;
        }
        if(p->state == parking_state_have_paid)
        {
            need_to_send_to_sever = 1;
            p->time = time_in_second; // second
            p->state = parking_state_have_paid_unlock_failed;
        }
        if(p->state == parking_state_unbooking)
        {
            need_to_send_to_sever = 1;
            p->time = time_in_second; // second
            p->state = parking_state_unbooking_unlock_failed;
        }
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
void set_online(unsigned short netaddr)
{
    int loop = 0;
    //pthread_mutex_lock(&parking_info_mutex);
    if(pstParkingState == NULL)
    {
        //pthread_mutex_unlock(&parking_info_mutex);
        return;
    }

    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        if(pstParkingState[loop].parking_id == netaddr)
        {
            pstParkingState[loop].online = 1;
            pstParkingState[loop].offline_time_out = time((time_t*)NULL);
            break;
        }
    }
    //pthread_mutex_unlock(&parking_info_mutex);
}

void set_node_online(unsigned char *macaddr)
{
#if 0
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
#else
    int loop = 0;
    pthread_mutex_lock(&parking_info_mutex);
    if(pstParkingState == NULL)
    {
        pthread_mutex_unlock(&parking_info_mutex);
        return;
    }

    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        if(memcmp(pstParkingState[loop].parking_mac_addr,macaddr,8) == 0)
        {
            pstParkingState[loop].state = parking_state_idle;
            pstParkingState[loop].online = 1;
            need_to_send_to_sever = 1;
            break;
        }
    }
    pthread_mutex_unlock(&parking_info_mutex);
#endif
}

int get_local_addr(unsigned char *local_addr,unsigned char* long_addr)
{
#if 0
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
#else
    int loop = 0;
    pthread_mutex_lock(&parking_info_mutex);
    if(pstParkingState == NULL)
    {
        pthread_mutex_unlock(&parking_info_mutex);
        return -1;
    }
    else
    {
        for(loop = 0;loop < depot_info.depot_size;loop ++)
        {
            if(memcmp(pstParkingState[loop].parking_mac_addr,long_addr,8) == 0)
            {
                pstParkingState[loop].netaddr = *(unsigned short*)local_addr;
                pthread_mutex_unlock(&parking_info_mutex);
                return 0;
            }
        }
        pthread_mutex_unlock(&parking_info_mutex);
        return -1;
    }
#endif
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
        if(netaddr == pstParkingState[loop].netaddr)
        {
            //pthread_mutex_unlock(&parking_info_mutex);
            return &pstParkingState[loop];
        }
    }
    //pthread_mutex_lock(&parking_info_mutex);
    return NULL;
}

pst_parkingState search_use_parking_id(unsigned short parking_id)
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
        if(parking_id == pstParkingState[loop].parking_id)
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
printf("mapping\r\n");
    for(loop = 0;loop < depot_info.depot_size;loop ++)
    {
        if(pstParkingState[loop].parking_id == parking_id)
        {
            memcpy(pstParkingState[loop].parking_mac_addr,macaddr,8);
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
        buf[loop] = pstParkingState[loop].state;
    }
    pthread_mutex_unlock(&parking_info_mutex);
    return loop;
}

int set_parking_state(unsigned short parking_id,unsigned char state)
{
    pst_parkingState p;

    printf("==========0x%04x,state is 0x%02x\r\n",parking_id,state); 
    pthread_mutex_lock(&parking_info_mutex);
    p = search_use_parking_id(parking_id);
    if((p == NULL) || (p->state == state))
    {
        pthread_mutex_unlock(&parking_info_mutex);
        return 1;
    }
    printf("==========0x%04x,state is 0x%02x\r\n",parking_id,state); 
    switch(state)
    {
        case parking_state_idle:
            if(p->state == parking_state_booking_lock_failed)
            {
                p->state = parking_state_idle;
                XBeePutCtlCmd(p->parking_mac_addr,p->netaddr,en_order_unlock);
            }
            break;
        case parking_state_booking:
            if((p->state == parking_state_idle) || (p->state == parking_state_unbooking_unlock) || (p->state == parking_state_booked_coming_unlock_goto_idle) || (p->state == parking_state_have_paid_unlock_vehicle_leave))
            {
                p->state = parking_state_booking;
                XBeePutCtlCmd(p->parking_mac_addr,p->netaddr,en_order_lock);
                p->time = time((time_t)NULL);
            }
            else
            {
                //p->state = parking_state_booking_busy;
            }
            break;
        case parking_state_booked_coming:
            if((p->state == parking_state_booking) || (p->state == parking_state_booking_lock))
            {
                p->state = parking_state_booked_coming;
                p->time = time((time_t)NULL);
                XBeePutCtlCmd(p->parking_mac_addr,p->netaddr,en_order_unlock);
            }
            else
            {
                //p->state = parking_state_booking_busy;
            }
            break;
        case parking_state_unbooking:
            if((p->state == parking_state_booking) 
               || (p->state == parking_state_booking_lock)
               //|| (p->state == parking_state_booking_busy) 
               || (p->state == parking_state_booking_lock_failed)
               //|| (p->state == parking_state_booked_coming_unlock)
               || (p->state == parking_state_booked_coming_unlock_failed)
               //|| (p->state == parking_state_booked_coming_lock)
               || (p->state == parking_state_booked_coming_lock_failed))
            {
                p->state = parking_state_unbooking;
                XBeePutCtlCmd(p->parking_mac_addr,p->netaddr,en_order_unlock);
                p->time = time((time_t)NULL);
            }
            else
            {
                
            }
            break;
        case parking_state_have_paid:
            switch(p->state)
            {
                case parking_state_stop_lock:
                    p->state = parking_state_have_paid;
                    p->time = time((time_t)NULL);
                    XBeePutCtlCmd(p->parking_mac_addr,p->netaddr,en_order_unlock);
                    break;
                case parking_state_booked_coming_lock:
                    p->state = parking_state_have_paid;
                    p->time = time((time_t)NULL);
                    XBeePutCtlCmd(p->parking_mac_addr,p->netaddr,en_order_unlock);
                    break;
                default:
                    break;
            }
        default:
            break;
    }
    
    pthread_mutex_unlock(&parking_info_mutex);
    return 1;
}
