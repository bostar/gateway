#ifndef __PARKING_STATE_MANAGEMENT_H__
#define __PARKING_STATE_MANAGEMENT_H__
#include <time.h>

typedef struct {
    unsigned short parking_id;
    unsigned char parking_mac_addr[8];
    unsigned char state;
    unsigned char online;
    unsigned char onlinecpy;
    en_parkingEvent event;
    en_parkingEvent vehicle_event;
    time_t vehicle_time;
    time_t time;
    time_t offline_time_out;
    unsigned char option;
}st_parkingState,*pst_parkingState;
pst_parkingState pstParkingState;
extern pthread_mutex_t parking_info_mutex;
extern unsigned short freetime;
extern unsigned short leavetime;
unsigned char need_to_send_to_sever;
int pkg(unsigned char *);
void set_depot_info(int depot_id,int depot_size,unsigned char wireless_channel,unsigned short net_id);
int get_depot_size(void);
int get_depot_id(void);
void parking_init(void);
void parking_state_check_routin(void);
void event_report(unsigned short netaddr,unsigned char event);
int networking_over(void);
void set_node_online(unsigned char *macaddr);
int get_local_addr(unsigned char *local_addr,unsigned char* long_addr);
pst_parkingState search_use_netaddr(unsigned short netaddr);
void parking_id_macaddr_mapping(unsigned short parking_id,unsigned char *macaddr,unsigned char option);
int get_all_parking_state(unsigned char* buf);
int set_parking_state(unsigned short parking_id,unsigned char state);
#endif
