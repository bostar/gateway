#ifndef _XBEE_LOG_H
#define _XBEE_LOG_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include<sys/types.h>
#include<sys/stat.h>

#include "xbee_vari_type.h"
#include "xbee_routine.h"
#include "xbee_api.h"
#include "xbee_bsp.h"
#include "xbee_atcmd.h"
#include "xbee_protocol.h"
#include "xbee_routine.h"
#include "xbee_api.h"
#include "server_duty.h"
#include "gpio.h"




int16 err_log(int8 *buf , uint16 len);

















#endif




