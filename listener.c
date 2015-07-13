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
    /* Get the Socket file descriptor */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("ERROR: Failed to obtain Socket Descriptor!\n");
        return;
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
}

int tcp_listen(unsigned char *revbuf,int bufsize)
{
    int num;
    bzero(revbuf,bufsize);
    num = recv(sockfd, revbuf, bufsize, 0);

    switch(num)
    {
        case -1:
            printf("ERROR: Receive string error!\n");
            close(sockfd);
            return (0);

        case  0:
            close(sockfd);
            return(0);

        default:
            printf ("OK: Receviced numbytes = %d\n", num);
            break;
    }

    revbuf[num] = '\0';
    printf ("OK: Receviced string is: %s\n", revbuf);
    return num;
}

int tcp_send_to_server(int len,unsigned char *bytes)
{
    int num;
    if((num = send(sockfd, bytes, len, 0)) == -1)
    {
        printf("ERROR: Failed to sent string.\n");
        close(sockfd);
        exit(1);
    }
    printf("OK: Sent %d bytes sucessful, please enter again.\n", num);
    return num;
}

void tcp_exit(void)
{
    close(sockfd);
}
