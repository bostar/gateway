#ifndef __XBEE_ROUTINE_H__
#define __XBEE_ROUTINE_H__
#include "xbee_vari_type.h"
#include "xbee_api.h"
#include "xbee_config.h"
#include <sys/queue.h>
#include "xbee_atcmd.h"

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
	uint8 nj;
}CoorInfoType;

struct XBeeDataWaiteSend
{
	APIFrameType APIFrame;
	
	uint8 mac_adr[8];
	uint8 net_adr[2];
	uint8 data[64];
	uint16 data_len;
	uint8 req;
	SetOptions options;
	TAILQ_ENTRY(XBeeDataWaiteSend)  tailq_entry;
};
#define XBeeDataWaiteSendType struct XBeeDataWaiteSend

CoorInfoType CoorInfo;
TAILQ_HEAD(,XBeeDataWaiteSend) waite_send_head;

extern SourceRouterLinkType *pLinkHead;
extern uint8 *HeadMidAdr;
extern uint8 rbuf[];
extern pthread_mutex_t xbee_mutex;
extern pthread_cond_t cond_send_xbee;
extern uint8 send_xbee_state;
extern uint32 waite_send_head_num;

void TestPrintf(int8* sss,int16 lens,uint8 *buf);
void xbee_routine_thread(void);
void xbee_routine_thread_send_data(void);
void xbee_routine_thread_timer(void);
void xbee_routine_thread_test(void);
void xbee_routine_thread_test_lar_node(void);

#endif
