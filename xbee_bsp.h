#ifndef __XBEE_BSP_H__
#define __XBEE_BSP_H__
#include "xbee_vari_type.h"
#include "xbee_atcmd.h"


#define SCAN_CHANNEL 0x4200
#define NJ_TIME 0xff

void xbee_gpio_init(void);
void xbee_serial_port_init(void);
int xbee_serial_port_read(unsigned char *buf);
int xbee_serial_port_write(unsigned char *buf,int len);
void xbee_gpio_set(int gpio,unsigned char level);
unsigned char xbee_gpio_get(int gpio);

void XBeeOpenBuzzer();
void XBeeCloseBuzzer();
void XBeeCreateNet(void);
int16 LeaveNetwork(void);




#endif