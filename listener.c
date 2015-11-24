/*
****************************************Copyright (c)**************************************************
**                               Guangzhou Zhiyuan Electronic Co.,LTD.
**                                     graduate school
**                                 http://www.zyinside.com
**
**------------------------------------- File Info ------------------------------------------------------
** File name:           listener.c
** Last modified Date:  2005-12-30
** Last Version:        1.0
** Descriptions:        listener of UDP.
**------------------------------------------------------------------------------------------------------
** Created by:          Ganda
** Created date:        2005-12-27
** Version:             1.0
** Descriptions:        Preliminary version.
**
**------------------------------------------------------------------------------------------------------
** Modified by:		Chenxibing
** Modified date:	2005-12-30	
** Version:		1.0.1
** Descriptions:	modified for MagicARM2410.
**
********************************************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <netdb.h>


struct sockaddr_in addr_server;
#define SERVER_IP	"121.43.69.176"
#define SERVER_PORT	8099                                  
#define PORT 8009                      // The port which is communicate with server
#define BACKLOG 10
#define LENGTH  512                    // Buffer length     

int sockfd;                                                                            
void tcp_init(void)
{ 
    struct sockaddr_in remote_addr;    // Host address information

    printf("%s\r\n",__func__);
    char *ptr,**pptr;
    struct hostent *hptr;
    char str[32];
    /* 取得命令后第一个参数，即要解析的域名或主机名 */
    /* 调用gethostbyname()。调用结果都存在hptr中 */
    if( (hptr = gethostbyname("test.starbo.com") ) == NULL )
    {
        printf("gethostbyname error for host:%s/n", ptr);
        return ; /* 如果调用gethostbyname发生错误，返回1 */
    }
    /* 将主机的规范名打出来 */
    printf("official hostname:%s\r\n",hptr->h_name);
    /* 主机可能有多个别名，将所有别名分别打出来 */
    for(pptr = hptr->h_aliases; *pptr != NULL; pptr++)
    printf(" alias:%s/n",*pptr);
    /* 根据地址类型，将地址打出来 */
    switch(hptr->h_addrtype)
    {
        case AF_INET:
            pptr=hptr->h_addr_list;
            /* 将刚才得到的所有地址都打出来。其中调用了inet_ntop()函数 */
            for(;*pptr!=NULL;pptr++)
            printf(" address:%s\r\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
            break;

        case AF_INET6:
            pptr=hptr->h_addr_list;
            /* 将刚才得到的所有地址都打出来。其中调用了inet_ntop()函数 */
            for(;*pptr!=NULL;pptr++)
            printf(" address:%s\r\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
            break;
        default:
            printf("unknown address type/n");
            break;
    }

    /* Get the Socket file descriptor */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("ERROR: Failed to obtain Socket Descriptor!\n");
        return;
    }

    /* Fill the socket address struct */
    remote_addr.sin_family = AF_INET;                   // Protocol Family
    remote_addr.sin_port = htons(SERVER_PORT);                 // Port number
    inet_pton(AF_INET, str, &remote_addr.sin_addr); // Net Address
    bzero(&(remote_addr.sin_zero), 8);                  // Flush the rest of struct

    /* Try to connect the remote */
    while (connect(sockfd, (struct sockaddr *)&remote_addr,  sizeof(struct sockaddr)) == -1)
    {
        printf ("ERROR: Failed to connect to the host!\n");
        usleep(1000000);
    }
    printf ("OK: Have connected to the server.\r\n");
}

int tcp_listen(unsigned char *revbuf,int bufsize)
{
    int num;
    static time_t time_in_second = 0;
    struct sockaddr_in remote_addr;    // Host address information
    bzero(revbuf,bufsize);
    num = recv(sockfd, revbuf, bufsize, MSG_DONTWAIT);

    switch(num)
    {
        case -1:
            //printf("ERROR: Receive string error!\n");
            //close(sockfd);
            //return (0);

        case  0:
            //close(sockfd);
            //return(0);

        default:
            //printf ("OK: Receviced numbytes = %d\n", num);
            break;
    }
    if(num > 0)
    {
        time_in_second = time((time_t *)NULL);
    }
    if(time((time_t *)NULL) - time_in_second > 1 * 5)//60 * 60)
    {
        close(sockfd);
        while((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            printf("ERROR: Failed to obtain Socket Descriptor!\n");
            usleep(1000000);
        }

        /* Fill the socket address struct */
        remote_addr.sin_family = AF_INET;                   // Protocol Family
        remote_addr.sin_port = htons(SERVER_PORT);                 // Port number
        inet_pton(AF_INET, SERVER_IP, &remote_addr.sin_addr); // Net Address
        bzero(&(remote_addr.sin_zero), 8);                  // Flush the rest of struct

        /* Try to connect the remote */
        while (connect(sockfd, (struct sockaddr *)&remote_addr,  sizeof(struct sockaddr)) == -1)
        {
            printf ("ERROR: Failed to connect to the host!\n");
            usleep(1000000);
        }
        printf ("OK: Have connected to the server.\r\n");
        time_in_second = time((time_t *)NULL);
    }
    revbuf[num] = '\0';
    //printf ("OK: Receviced string is: %s\n", revbuf);
    return num;
}

int tcp_send_to_server(int len,unsigned char *bytes)
{
    int num;
    if((num = send(sockfd, bytes, len, MSG_DONTWAIT )) == -1)
    {
        printf("ERROR: Failed to sent string.\n");
        //close(sockfd);
        exit(1);
    }
    //printf("OK: Sent %d bytes sucessful\n", num);
    return num;
}

void tcp_exit(void)
{
    close(sockfd);
}
