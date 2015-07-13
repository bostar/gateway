#ifndef	__LISTENER_H__
#define __LISTENER_H__

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

struct sockaddr_in addr_server;
void tcp_init(void);
int tcp_listen(unsigned char *revbuf,int bufsize);
int tcp_send_to_server(int len,unsigned char *bytes);
void tcp_exit(void);
#endif
