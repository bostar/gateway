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
#include "xbee_routine.h"

#define ttyO0  0
#define ttyO1  1
#define ttyO2  2
#define ttyO3  3
#define ttyO4  4
#define ttyO5  5

int fd1,fd2,fd3,fd4,fd5;
static int n_com_port = ttyO1;

void xbee_gpio_init(void)
{
    int ret;
    fd1 = open("/sys/class/gpio/gpio53/value",O_RDWR);
    if(fd1 < 0)
    {
        printf("Open gpio53 failed!\r\n");
    }
    fd2 = open("/sys/class/gpio/gpio55/value",O_RDWR);
    if(fd2 < 0)
    {
        printf("Open gpio55 failed!\r\n");
    }
    fd3 = open("/sys/class/gpio/gpio57/value",O_RDWR);
    if(fd3 < 0)
    {
        printf("Open gpio57 failed!\r\n");
    }
    fd4 = open("/sys/class/gpio/gpio59/value",O_RDWR);
    if(fd4 < 0)
    {
        printf("Open gpio59 failed!\r\n");
    }
    fd5 = open("/sys/class/gpio/gpio45/value",O_RDWR);
    if(fd5 < 0)
    {
        printf("Open gpio45 failed!\r\n");
    }

    ret = write(fd1, "1", 1); // DEF
    ret = write(fd2, "1", 1); // SLEEP
    ret = write(fd3, "1", 1); // WAKEUP
    ret = write(fd4, "1", 1); // RESET
}
void xbee_serial_port_init(void)
{
    int ret = -1;

    ret = OpenComPort(n_com_port, 9600, 8, "1", 'N');
    if (ret < 0) {
        fprintf(stderr, "Error: Opening Com Port %d\n", n_com_port);
        return;
    }else{
        printf("Open Com Port %d Success, Now begin work\n", n_com_port);
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

void xbee_gpio_set(int gpio,unsigned char level)
{

}

unsigned char xbee_gpio_get(int gpio)
{
    return 0;
}

/*******************************************
**brief 创建网络
*******************************************/
void XBeeCreateNet(void)
{
	uint8 panID[8],i;
	XBeeSendAT("RE");
	usleep(1000);
	LeaveNetwork();	
	usleep(100000);
	for(i=0;i<8;i++)
		panID[i] = 0x00;
	XBeeSetPanID(panID,NO_RES);   //设置ID的值	
	XBeeSetChannel(SCAN_CHANNEL,NO_RES); //设置信道
	XbeeSendAC(NO_RES);
	XBeeSendWR(NO_RES);
}
/*******************************************
**brief 离开网络
*******************************************/
int16 LeaveNetwork(void)
{
	uint8 param=4;
	int8 *cmd;
	cmd = "CB";
	return XBeeSetAT(cmd, &param, 1, NO_RES);
}






































