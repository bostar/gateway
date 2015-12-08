#ifndef __XBEE_BSP_H__
#define __XBEE_BSP_H__
#include "xbee_vari_type.h"
#include "xbee_atcmd.h"
#include "stdbool.h"




#define NJ_TIME 0xff


extern int gpio_xbee_rts,gpio_sleep_ctl,fd3,gpio_xbee_reset,gpio_xbee_cts;


void xbee_gpio_init(void);
bool xbee_gpio_set(int gpio,unsigned char level);
unsigned char xbee_gpio_get(int gpio);

void xbee_serial_port_init(uint32 bd);
void xbee_close_serial_port(void);
int xbee_serial_port_read(unsigned char *buf);
int xbee_serial_port_write(unsigned char *buf,int len);

void XBeeOpenBuzzer();
void XBeeCloseBuzzer();
void XBeeCreateNet(void);
int16 LeaveNetwork(void);
bool read_xbee_cts(char *buf);
void xbee_reset(void);





#endif
