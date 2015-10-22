#ifndef __XBEE_ROUTINE_H__
#define __XBEE_ROUTINE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <string.h>
#include "stdbool.h"
#include <unistd.h>
#include <time.h>
#include <sys/queue.h>
#include "stdarg.h"
#include "server_duty.h"
#include "xbee_vari_type.h"
#include "xbee_bsp.h"
#include "xbee_atcmd.h"
#include "xbee_protocol.h"
#include "xbee_test.h"
#include "xbee_protocol.h"
#include "xbee_config.h"
#include "xbee_api.h"

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



/**************************declarations gloable variable************************/
CoorInfoType CoorInfo;
TAILQ_HEAD(,XBeeDataWaiteSend) waite_send_head;
extern SourceRouterLinkType *pLinkHead;
extern CircularQueueType serial_rbuf;
extern uint8 *HeadMidAdr;
extern pthread_mutex_t xbee_mutex;
extern pthread_cond_t cond_send_xbee;
extern uint32 send_xbee_state;
extern uint32 waite_send_head_num;
extern CircularQueueType trans_status_buf;
extern CircularQueueType xbee_rev_buf;
extern CircularQueueType xbee_send_buf;
extern CircularQueueType trans_req_buf;
/**************************declarations of mutex************************/
extern pthread_mutex_t mutex01_serial_rbuf;
extern pthread_mutex_t mutex02_pLinkHead;
extern pthread_mutex_t mutex03_send_xbee_state;
extern pthread_mutex_t mutex04_waite_send_head_num;
//extern pthread_mutex_t mutex05_send_data_timeout;
extern pthread_mutex_t mutex06_waite_send_head;
extern pthread_mutex_t mutex07_CoorInfo;
extern pthread_mutex_t mutex08_trans_status_buf;
extern pthread_mutex_t mutex09_xbee_rev_buf;
extern CircularQueueType xbee_send_buf;
extern pthread_mutex_t mutex10_xbee_send_buf;
extern pthread_mutex_t mutex11_serial_port;
extern pthread_mutex_t mutex12_trans_req_buf;

void TestPrintf(int8* sss,int16 lens,uint8 *buf);
void xbee_routine_thread(void);
void xbee_routine_thread_write_serial(void);
void xbee_routine_thread_read_serial(void);
void xbee_routine_thread_process_serial_buf(void);
void xbee_routine_thread_serial_data_process(void);
void xbee_routine_thread_process_trans_status_buf(void);
void xbee_routine_thread_read_trans_req_buf(void);

void xbee_routine_thread_test(void);
void xbee_routine_thread_test_lar_node(void);



#endif
