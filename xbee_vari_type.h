#ifndef _xbee_vari_type_h_
#define _xbee_vari_type_h_

#include <pthread.h>

/*
#define uint8 unsigned char
#define uint32 unsigned int
#define int8 char
#define int32 int
#define int16 short int
#define uint16 unsigned short int
#define uint64 unsigned long long
#define int64 long long
*/

typedef unsigned char uint8;
typedef unsigned int  uint32;
typedef char int8;
typedef int  int32;
typedef short int int16;
typedef unsigned short int uint16;
typedef unsigned long long uint64;
typedef long long int64;

#define MUTEX_LOCK(x)		while(pthread_mutex_lock((x)) != 0)
#define MUTEX_UNLOCK(x)		while(pthread_mutex_unlock((x)) != 0)





#endif
