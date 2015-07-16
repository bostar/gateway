#ifndef __XBEE_BSP_H__
#define __XBEE_BSP_H__

void xbee_gpio_init(void);
void xbee_serial_port_init(void);
int xbee_serial_port_read(unsigned char *buf);
int xbee_serial_port_write(unsigned char *buf,int len);
void xbee_gpio_set(int gpio,unsigned char level);
unsigned char xbee_gpio_get(int gpio);

void XBeeOpenBuzzer();
void XBeeCloseBuzzer();

#endif
