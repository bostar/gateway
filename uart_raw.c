#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <limits.h>
#define DEV_NAME		"/dev/ttyO1"

int iFd;

void uart_init(void)
{
        struct termios opt;

        iFd = open(DEV_NAME, O_RDWR | O_NOCTTY);

        if(iFd < 0) {
                perror(DEV_NAME);
                return;
        }

        tcgetattr(iFd, &opt);
        cfsetispeed(&opt, B115200);
        cfsetospeed(&opt, B115200);

        if (tcgetattr(iFd,   &opt)<0) {
		printf("uart set attr err!\r\n");
                  return;
        }
        opt.c_lflag	&= ~(ECHO | ICANON | IEXTEN | ISIG);
        opt.c_iflag	&= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        opt.c_oflag 	&= ~(OPOST);
        opt.c_cflag 	&= ~(CSIZE | PARENB);
        opt.c_cflag 	|=  CS8;

        opt.c_cc[VMIN] 	= 5;
        opt.c_cc[VTIME]	= 10;

        if (tcsetattr(iFd,   TCSANOW,   &opt)<0) {
		printf("uart cfg err!\r\n");
            	return;
        }
        tcflush(iFd,TCIOFLUSH);
        printf("uart cfg ok\r\n");
        return;
}

void uart_write(unsigned char *buf,int len)
{
    write(iFd,buf,len);
}

int uart_read(unsigned char *buf)
{
    int len;
    len = read(iFd,buf,0xff);
    if(len == 0)
        return;
    printf("uart get data:");
    printf("%d,%s\r\n",len,buf);
    return len;
}

void uart_test(void)
{
    int len,i;
    unsigned char ucBuf[1000];
    for (i = 0; i < 1000; i++){
        ucBuf[i] = 0xff - i;
    }

    write(iFd, ucBuf, 0xff);

    len = read(iFd, ucBuf, 0xff);
    printf("get data: %d \n", len);
    for (i = 0; i < len; i++){
        printf(" %x", ucBuf[i]);
    }
    printf("\n");
}

void uart_exit(void)
{
    close(iFd);
}
