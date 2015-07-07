#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <limits.h>

int fd1,fd2,fd3,fd4,fd5;
void gpio_init(void)
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

    /*ret = read(gpio_fd,buff,10);
    if( ret == -1 )
        err_print("read");
    */
}

void sleep_zm516x(unsigned char state)
{
	int ret;
    if(state)
    {
        ret = write(fd2, "0", 1); // SLEEP
        ret = write(fd3, "1", 1); // WAKEUP
    }
    else
    {
        ret = write(fd2, "1", 1); // SLEEP
        ret = write(fd3, "0", 1); // WAKEUP
    }
}

void reset_params_zm516x(void)
{
	int ret;
	
    ret = write(fd1, "1", 1); // DEF
}
void reset_zm516x(void)
{
	int ret;
    ret = write(fd4, "0", 1);
    ret = write(fd1, "0", 1); // DEF
    usleep(100000);
    ret = write(fd4, "1", 1);
    usleep(100000);
    printf("reset zm516x\r\n");
}
