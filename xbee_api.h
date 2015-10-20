#ifndef _xbee_api_h_
#define _xbee_api_h_

#include "xbee_vari_type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xbee_protocol.h"

typedef struct _SourceRouterLinkType
{
	uint16 target_adr;
	uint8 mac_adr[8];
	uint8 mid_adr[40];
	uint8 num_mid_adr;
	uint8 dev_type;
	LockStateType lock_state;
	uint32 send_cmd_times;
	uint32 rev_rep_times;
	uint16 join_net_times;
	struct _SourceRouterLinkType *next;

}SourceRouterLinkType;






SourceRouterLinkType *CreatRouterLink(uint8 *mac_adr,uint16 target_adr,uint8 *mid_adr,uint8 num);
SourceRouterLinkType *FindNetAdr(const SourceRouterLinkType *pNode,uint16 target_adr);
SourceRouterLinkType *FindMacAdr(const SourceRouterLinkType *pNode,uint8 *mac_adr);
uint8 AddData(const SourceRouterLinkType *pNode,SourceRouterLinkType *pNodeS);
uint16 LinkLenth(const SourceRouterLinkType *pNode);
uint8 DeleteNode(const SourceRouterLinkType *pNode,SourceRouterLinkType *deleteNode);
uint8 compareNode(SourceRouterLinkType *pNode,SourceRouterLinkType *pNodeS);
void NodePrintf(SourceRouterLinkType *pNode);
void LinkPrintf(SourceRouterLinkType *pNode);
int8 arrncmp(uint8 *arr1,uint8 *arr2,uint8 n);
SourceRouterLinkType *CreatNode(uint8 *mac_adr,uint8 *target_adr);
SourceRouterLinkType *FindnNode(const SourceRouterLinkType *pNode,uint8 n);


#endif
