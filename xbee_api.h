#ifndef _xbee_api_h_
#define _xbee_api_h_

#include "xbee_vari_type.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct _SourceRouterLinkType
{
	uint16 target_adr;
	uint8 mid_adr[40];
	uint8 num_mid_adr;
	struct _SourceRouterLinkType *next;
}SourceRouterLinkType;




















#endif
