#ifndef __SERVER_DUTY_H__
#define __SERVER_DUTY_H__



typedef enum{
    en_vehicle_comming,
    en_vehicle_leave,
    en_lock_success,
    en_lock_failed,
    en_unlock_success,
    en_unlock_failed,
    en_max_event
}en_parkingEvent,*pen_parkingEvent;

typedef enum{
    en_order_lock = 0x01,
    en_order_unlock = 0x00
}en_parking_order;

int get_local_addr(unsigned char *local_addr,unsigned char* long_addr);
void server_duty_thread(void);
void set_node_online(unsigned char *macaddr);
int networking_over(void);
void event_report(unsigned short netaddr,unsigned char event);
void get_channel_panid(unsigned char* channel,unsigned short*panid);

#endif
