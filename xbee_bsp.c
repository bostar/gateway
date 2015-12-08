/*****************************************************************************************************
**name	xbee_bsp.c
**brief	主要包括板级功能函数，串口初始化，xbee网络初始化等
**
*****************************************************************************************************/

#include "xbee_bsp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <limits.h>
#include "serial.h"
#include "xbee_config.h"


#define ttyO0  0
#define ttyO1  1
#define ttyO2  2
#define ttyO3  3
#define ttyO4  4
#define ttyO5  5

int gpio_xbee_rts,gpio_sleep_ctl,fd3,gpio_xbee_reset,gpio_xbee_cts;
static int n_com_port = ttyO1;

/***************io口操作***********************************************************************************/
void xbee_gpio_init(void)
{
    int ret;
    gpio_xbee_rts = open("/sys/class/gpio/gpio53/value",O_RDWR);
    if(gpio_xbee_rts < 0)
    {
        printf("Open gpio53 failed!\r\n");
    }
    gpio_sleep_ctl = open("/sys/class/gpio/gpio55/value",O_RDWR);
    if(gpio_sleep_ctl < 0)
    {
        printf("Open gpio55 failed!\r\n");
    }
    fd3 = open("/sys/class/gpio/gpio57/value",O_RDWR);
    if(fd3 < 0)
    {
        printf("Open gpio57 failed!\r\n");
    }
    gpio_xbee_reset = open("/sys/class/gpio/gpio59/value",O_RDWR);
    if(gpio_xbee_reset < 0)
    {
        printf("Open gpio59 failed!\r\n");
    }
    gpio_xbee_cts = open("/sys/class/gpio/gpio45/value",O_RDONLY);
    if(gpio_xbee_cts < 0)
    {
        printf("Open gpio45 failed!\r\n");
    }

    ret = write(gpio_xbee_rts, "1", 1); // RTS
    ret = write(gpio_sleep_ctl, "1", 1); // SLEEP
    ret = write(fd3, "1", 1);
    ret = write(gpio_xbee_reset, "1", 1); // RESET
}

bool xbee_gpio_set(int gpio,unsigned char level)
{
	int8 buf;
	int res=-1;

	buf = (int8)level;
	res = lseek(gpio, 0, SEEK_SET);
	if(res < 0)
	{
		printf("\033[31mwrite gpio failed!\033[0m\r\n");
	}
	res = write(gpio , &buf , 1);
	if(res < 0)
	{
		printf("\033[31mwrite gpio failed!\033[0m\r\n");
		return false;
	}
	return true;
}

unsigned char xbee_gpio_get(int gpio)
{
	uint8 buf[1];

	lseek(gpio_xbee_rts, 0, SEEK_SET);
	if(read(gpio_xbee_rts , buf , 1) < 0)
	{
		printf("\033[31mread gpio failed!\033[0m\r\n");
		return 2;
	}
	if(*buf == '0')
		return 0;
	else if(*buf == '1')
		return 1;
	else
		return 2;
}
/***************串口操作*****************************************************************************************/
void xbee_serial_port_init(uint32 bd)
{
    int ret = -1;
	static uint8 count=0;

	if(count> 0)
	{
		CloseComPort();
	}
	count = 1;
    ret = OpenComPort(n_com_port, bd, 8, "1", 'N');
    if (ret < 0) 
	{
        fprintf(stderr, "Error: Opening Com Port %d\n", n_com_port);
        return;
    }
	else
	{
        printf("Open Com Port %d Success.BD is %d, Now begin work\n", n_com_port , bd);
    }
}

void xbee_close_serial_port(void)
{
	int32 ret = -1;

	ret = close(n_com_port);
	if(ret < 0)
	{
		fprintf(stderr, "Error: close Com Port %d\n", n_com_port);
        return;
	}
	else
	{
		printf("close Com Port %d Success\n", n_com_port);
	}
}

int xbee_serial_port_read(unsigned char *buf)
{
    return ReadComPort(buf,255);
}

int xbee_serial_port_write(unsigned char *buf,int len)
{
    return WriteComPort(buf, len);
}
/**************xbee操作***************************************************************************************/
/*******************************************
**brief cts control
*******************************************/
bool read_xbee_cts(char *buf)
{
	int res=-1;

	lseek(gpio_xbee_cts, 0, SEEK_SET);
	res = read(gpio_xbee_cts , buf , 1);
	if(res == 1)
	{
		return true;
	}
	return false;
}
/*******************************************
**brief xbee reset
*******************************************/
void xbee_reset(void)
{
	bool state=false;

	do{
		state = xbee_gpio_set(gpio_xbee_reset, (uint8)'0');
		usleep(100000);
	}while(state == false);
	printf("\033[33mxbee reset\033[0m\r\n");
	usleep(100000);
	state = false;
	do{
		state = xbee_gpio_set(gpio_xbee_reset, (uint8)'1');
		usleep(100000);
	}while(state == false);
	printf("\033[33mxbee start\033[0m\r\n");
}


































