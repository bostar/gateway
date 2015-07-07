#ifndef	__LISTENER_H__
#define __LISTENER_H__

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

struct sockaddr_in addr_server;
void udp_init(void);
int udp_listen(unsigned char *revbuf,int bufsize);
int udp_send_to_server(int len,unsigned char *bytes);
void udp_exit(void);
#endif
