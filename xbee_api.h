#ifndef _xbee_api_h_
#define _xbee_api_h_

#include "xbee_vari_type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "xbee_protocol.h"
#include "xbee_config.h"
#include "stdbool.h"
//#include "xbee_routine.h"
#include <sys/queue.h>


#define S_FILE_TITLE	"park_id  net_adr mac_adr               senerx senery senerz"
#define S_FILE_TITLE1	"park_id  "
#define S_FILE_TITLE2	"net_adr "
#define S_FILE_TITLE3	"mac_adr            "
#define S_FILE_TITLE4	"senerx "
#define S_FILE_TITLE5	"senery "
#define S_FILE_TITLE6	"senerz"


typedef struct _SourceRouterLinkType
{
	uint16 target_adr;
	uint8 mac_adr[8];
	uint8 mid_adr[40];
	uint8 num_mid_adr;
	uint8 cnt;
	struct _SourceRouterLinkType *next;
}SourceRouterLinkType;

typedef struct _LockListType
{
	uint16 park_id;
	uint8 net_adr[2];
	uint8 mac_adr[8];
	int16 statex;
	int16 statey;
	int16 statez;
	struct _LockListType *next;
}LockListInfoType;

typedef struct
{
	uint32 front;
	uint32 rear;
	uint32 count;
	uint32 maxsize;
	uint8 elements[_REV_DATA_MAX];
}CircularQueueType;

typedef struct _a
{
	uint8 mac_adr[8];
	uint8 net_adr[2];
	uint8 data[10];
	uint8 len;
	bool state;
	struct _a *next;
}transReqListType;





SourceRouterLinkType *CreatRouterLink(void);
SourceRouterLinkType *CreatRouterLinkNode(uint8 *mac_adr,uint8 *target_adr,uint8 *mid_adr,uint8 num);
SourceRouterLinkType *FindNetAdr(SourceRouterLinkType *pNode,uint16 target_adr);
SourceRouterLinkType *FindMacAdr(SourceRouterLinkType* const pNode,uint8 *mac_adr);
SourceRouterLinkType *FindnNode(SourceRouterLinkType* const pNode,uint16 n);
uint8 AddData(SourceRouterLinkType* const pNode,SourceRouterLinkType *pNodeS);
uint16 LinkLenth(SourceRouterLinkType* const pNode);
uint8 DeleteNode(SourceRouterLinkType * const pNode,SourceRouterLinkType *deleteNode);
uint8 compareNode(SourceRouterLinkType *pNode,SourceRouterLinkType *pNodeS);
void NodePrintf(SourceRouterLinkType *pNode);
void LinkPrintf(SourceRouterLinkType *pNode);

LockListInfoType *creat_lock_list_info(void);
LockListInfoType *creat_lock_list_info_node(void);
LockListInfoType *find_node_mac(LockListInfoType* const Node,uint8 *mac_adr);
LockListInfoType *find_node_park_id(LockListInfoType* const Node,uint16 park_id);
uint8 add_node(LockListInfoType* const pNode,LockListInfoType *pNodeS);
uint8 del_lock_list_node(LockListInfoType* const pNode,LockListInfoType *deleteNode);
uint16 lock_list_len(LockListInfoType* const pNode);
int write_lock_list(LockListInfoType* const pNode);
int read_lock_list(LockListInfoType* const pNode);
void xbee_locker_list_update(LockListInfoType* const pNode , uint16 park_id , uint8 *mac_adr);

void creat_circular_queue( CircularQueueType *queue );
bool is_empty( CircularQueueType *queue);
bool is_full( CircularQueueType *queue );
bool in_queue( CircularQueueType *queue, uint8 value);
bool out_queue( CircularQueueType *queue , uint8 *out_buf);
void print_queue(CircularQueueType *queue);
uint16 read_cqueue(CircularQueueType* p_cqueue , uint8* buf , uint16 n);
uint16 write_cqueue(CircularQueueType* p_cqueue , uint8* buf , uint16 n);
void clear_queue(CircularQueueType *queue);

transReqListType *creat_trans_req_node(void);
transReqListType *creat_trans_req_list(void);

int8 arrncmp(uint8 *arr1,uint8 *arr2,uint8 n);
uint32 arr2int(uint8 *buf , uint8 len);
void str2hex(char *str , uint8 len , uint8 *arr);
int32 str2dec(char *str , uint8 len);











#endif
