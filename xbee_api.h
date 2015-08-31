#ifndef _xbee_api_h_
#define _xbee_api_h_

#include "xbee_vari_type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _SourceRouterLinkType
{
	uint16 target_adr;
	uint8 mid_adr[40];
	uint8 num_mid_adr;
	struct _SourceRouterLinkType *next;
}SourceRouterLinkType;






SourceRouterLinkType *CreatRouterLink(uint16 target_adr,uint8 *mid_adr,uint8 num);
SourceRouterLinkType *FindData(const SourceRouterLinkType *pNode,uint16 target_adr);
uint8 AddData(const SourceRouterLinkType *pNode,SourceRouterLinkType *pNodeS);
uint16 LinkLenth(const SourceRouterLinkType *pNode);
uint8 DeleteNode(const SourceRouterLinkType *pNode,SourceRouterLinkType *deleteNode);
uint8 compareNode(SourceRouterLinkType *pNode,SourceRouterLinkType *pNodeS);
void NodePrintf(SourceRouterLinkType *pNode);
void LinkPrintf(SourceRouterLinkType *pNode);




#endif
