#ifndef __XBEE_PROTOCOL_H__
#define __XBEE_PROTOCOL_H__

#include "xbee_vari_type.h"
#include <unistd.h>
#include <stdio.h>



typedef enum
{
	lock_event	=	0x01,
	bat_event	=	0x02
}SENcmdType;

typedef enum
{
	net_request	=	0x01,
}CFGcmdType;

typedef enum
{
	lock_ctl		=	0x00,
	lock_response	=	0x01
}CTLcmdType;


typedef enum     //SEN锁事件上报类型
{
    ParkingUsed             =   0x01,   //车位被使用
    ParkingUnUsed           =   0x02,   //车位空
    ParkLockSuccess         =   0x03,   //锁定成功
    ParkLockFailed          =   0x04,   //锁定失败
    ParkUnlockSuccess       =   0x05,   //解锁成功
    ParkUnlockFailed        =   0x06,   //解锁失败
    ParkLockingOrUnlocking  =   0x07
} parkingEventType;



int16 UartRevDataProcess(uint8* UartRevBuf);
int16 XBeeJionEnable(uint8 *ieeeadr,uint8 *netadr);
int16 XBeeJionDisable(uint8 *ieeeadr,uint8 *netadr);
void XBeeProcessCFG(uint8 *rbuf);
void XBeeProcessCTL(uint8 *rbuf);
void XBeeProcessSEN(uint8 *rbuf);
void XBeeProcessRoutRcord(uint8 *rbuf);
void ProcessND(uint8 *rbuf);
int16 XBeeSendSenserInit(uint8 *ieeeadr,uint8 *net_adr);
int16 XBeeEndDeviceLock(uint8 *ieeeadr,uint8 *netadr);
int16 XBeeEndDeviceUnlock(uint8 *ieeeadr,uint8 *netadr);
int16 XBeePutCtlCmd(uint8 *ieeeadr,uint16 netadr,uint8 lockstate);
int16 putCtlCmd(uint16 netadr,uint8 lockstate);
int16 XBeeSendTimeout(uint8 time);
uint16 char_to_int(uint8 *data);



#endif
