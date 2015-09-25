#ifndef __XBEE_ROUTINE_H__
#define __XBEE_ROUTINE_H__
#include "xbee_vari_type.h"
#include "xbee_api.h"
#include "xbee_config.h"

#define NO_NET        1
#define IN_NET        2
 

typedef enum
{
	ReqJion     	 =	 0x01,
	AllowJion		 =	 0x02,
	ReFactory		 =	 0x03
}CREprotocolType;

typedef struct 
{
	uint8 NetState;
	uint8 ARper;
	uint8 CloseNet;
	uint16 panID16;
	uint8 panID64[8];
	uint8 channel;
}CoorInfoType;

CoorInfoType CoorInfo;

extern SourceRouterLinkType *pLinkHead;
extern uint8 *HeadMidAdr;


extern uint8 rbuf[];



void TestPrintf(int8* sss,int16 lens,uint8 *buf);
void xbee_routine_thread(void);
void xbee_routine_thread_test(void);

#endif
