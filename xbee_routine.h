#ifndef __XBEE_ROUTINE_H__
#define __XBEE_ROUTINE_H__
#include "xbee_vari_type.h"


#define NO_NET        1
#define IN_NET        2

typedef enum
{
	ReqJion   	 =	 0x01,
	AllowJion	 =	 0x02,
	ReFactory	 =	 0x03
}CREprotocolType;






extern uint8 rbuf[];




void xbee_routine_thread(void);
void xbee_routine_thread_test(void);

#endif
