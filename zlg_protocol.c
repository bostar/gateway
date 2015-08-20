#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "uart_raw.h"
#include "zlg_protocol.h"
#include "zlg_cmd.h"
#include "serial.h"
#include "server_duty.h"
#include "ctl_cmd_cache.h"

unsigned char iEEEAddress[8];
unsigned short requestAddress;
const unsigned short addresses[20];

void mac2str(char *str,const char *ieeeAddress);


void communicate_thread(void)
{
	unsigned char rbuf[255];
	unsigned char rlen;
	unsigned short allocLocalAddress;
	unsigned char allocChannel;
	unsigned short allocPanid;
	unsigned char ctl_cmd;
	char macstr[20];

	set_temporary_cast_mode(broadcast);
    usleep(100000);

	while(1)
	{
		pthread_mutex_lock(&mut);
		rlen = ReadComPort(rbuf,255);
		pthread_mutex_unlock(&mut);
		if(rlen)
		{
            printf("recevie form node %d bytes\r\n",rlen);
			if(rbuf[0] == 'C' && rbuf[1] == 'F' && rbuf[2] == 'G')
			{
				switch(rbuf[3])
				{
					case cmdCheckIn:
						memcpy(&iEEEAddress,&rbuf[4],8);
						mac2str(macstr,(const char *)iEEEAddress);
                        printf("check in node ieee address is:0x%s\r\n",macstr);
                        if(!get_local_addr((unsigned char *)&allocLocalAddress,(unsigned char *)&iEEEAddress))
						{
							get_channel_panid(&allocChannel,&allocPanid);
							ackRegisterNetwork(allocLocalAddress,Allow,0x00,15);
							//ackRegisterNetwork(allocLocalAddress,Allow,allocPanid,allocChannel);
                            usleep(100000);
						    printf("server alloc node address:0x%04x success\r\n",allocLocalAddress);
							set_node_online((unsigned char *)&iEEEAddress);
							if(networking_over())
							{
                                printf ("***********************networking_over...\r\n!n");
                            } 
						}
						else
							printf("server can not alloc address\r\n");
					break;
					case cmdAckLinkTest:
						memcpy(&iEEEAddress,&rbuf[4],8);
						mac2str(macstr,(const char *)iEEEAddress);
						printf("0x%s LinkTest ACK...\r\n",macstr);
					break;
					case cmdDataRequest:
						requestAddress = (unsigned short)rbuf[4] << 8 | rbuf[5];
//						printf("node 0x%04x is requesting data...\r\n",requestAddress);
						if(!getCtlCmd(requestAddress,&ctl_cmd))
							switchLockControl(requestAddress,ctl_cmd);
						else
							printf("cache not exist node 0x%04x ctl_cmd\r\n",requestAddress);
					break;
					default:
					break;
				}
			}
			if(rbuf[0] == 'S' && rbuf[1] == 'E' && rbuf[2] == 'N')
			{
				switch(rbuf[3])
				{
					case cmdEventReport:
				        requestAddress = (unsigned short)rbuf[5] << 8 | rbuf[6];
                        ackEventReport(requestAddress);
                        printf("node 0x%04x is reporting event...\r\n",requestAddress);
					 	event_report(requestAddress,rbuf[4]);
					break;
					case cmdBatteryRemainReport:
				        requestAddress = (unsigned short)rbuf[5] << 8 | rbuf[6];
					break;
                    case 0x03:
				        requestAddress = (unsigned short)rbuf[6] << 8 | rbuf[7];
                        printf("-------------------node 0x%04x: voltage is:0x%04x\r\n",requestAddress,rbuf[4] << 8| rbuf[5]);
                    break;
					default:
					break;
				}
			}
			memset(rbuf,0x0,rlen);
		}
        usleep(10000);
	}
}

void mac2str(char *str,const char *ieeeAddress)
{
	int i;
   	char str2[3];
	strcpy(str,"");
	for(i = 0;i < 8;i++)
    	{
        	sprintf(str2,"%02x",*ieeeAddress++);
        	strcat(str,str2);
    	}
	return;
}

void ackRegisterNetwork(unsigned short NetAddress,ackCmd_t cmd,unsigned short panid,unsigned char channel)
{
	char wbuf[18];
	char str[20];
	unsigned char *ieeeAddress = iEEEAddress;

	wbuf[0] = 'C';
	wbuf[1] = 'F';
	wbuf[2] = 'G';	
	wbuf[3] = cmdAckCheckIn;	
	memcpy(&wbuf[4],(const char *)ieeeAddress,8);
	wbuf[12] = cmd;
	wbuf[13] = NetAddress >> 8;
	wbuf[14] = NetAddress;
	wbuf[15] = panid >> 8;
	wbuf[16] = panid;
	wbuf[17] = channel;

// 	set_temporary_DestAddr(0xfffe);
//	set_temporary_cast_mode(unicast);
    usleep(100000);
	WriteComPort((unsigned char *)wbuf, 18);

	mac2str(str,(const char *)iEEEAddress);
	if(cmd == Allow)
		printf("have ack 0x%s allocate local address is 0x%04x allow,panid is:0x%04x ,channel is:%d\r\n",str,NetAddress,panid,channel);
	else
		printf("have ack 0x%s allocate local address is 0x%04x refuse...\r\n",str,NetAddress);
}

void testLink(const char * ieeeAddress)
{
	unsigned char wbuf[12];
	char str[20];
	
	wbuf[0] = 'C';
	wbuf[1] = 'F';
	wbuf[2] = 'G';
	wbuf[3] = cmdLinkTest;
	memcpy(&wbuf[4],ieeeAddress,8);
	
//	set_temporary_cast_mode(unicast);
//    usleep(100000);
	WriteComPort( wbuf, 12);
	
	mac2str(str,ieeeAddress);
	printf("taking link test to 0x%s...\r\n",str);
}

void startSensorCalibration(void)
{
	unsigned char wbuf[4];
	
	wbuf[0] = 'S';
	wbuf[1] = 'E';
	wbuf[2] = 'N';
	wbuf[3] = cmdSensorCalibration;
	
//	set_temporary_cast_mode(broadcast);
//    usleep(100000);
	WriteComPort(wbuf, 4);
	
	printf("taking sensor calibration...\r\n"); 
}

void ackEventReport(unsigned short DstAddr)
{
    unsigned char wbuf[7];
    
    wbuf[0] = 'S';
    wbuf[1] = 'E';
    wbuf[2] = 'N';
    wbuf[3] = 0x04;//cmd
    wbuf[4] = 0x00;//success 
    wbuf[5] = DstAddr >> 8;
    wbuf[6] = DstAddr;

//    set_temporary_DestAddr(DstAddr);
//	set_temporary_cast_mode(unicast);
//    usleep(100000);
	WriteComPort(wbuf, 7);
    printf("have acked node 0x%04x event report_______\r\n",DstAddr);
}

void testBeep(unsigned short DstAddr,unsigned char cmd)
{
	unsigned char wbuf[7];
	
	wbuf[0] = 'T';
	wbuf[1] = 'S';
	wbuf[2] = 'T';
	wbuf[3] = cmdBeepTest;
	wbuf[4] = cmd;
    wbuf[5] = DstAddr >> 8;
    wbuf[6] = DstAddr;
	
//	set_temporary_DestAddr(DstAddr);
//	set_temporary_cast_mode(unicast);
//    usleep(100000);
	WriteComPort(wbuf, 7);
	
	if(cmd == cmdSilence)
		printf("node 0x%04x beep start to silence...\r\n",DstAddr);
	else
		printf("node 0x%04x beep start to buzzing...\r\n",DstAddr);
}

void testLed(unsigned short DstAddr,unsigned char ioLevel)
{
	unsigned char wbuf[7];
	
	wbuf[0] = 'T';
	wbuf[1] = 'S';
	wbuf[2] = 'T';
	wbuf[3] = cmdLedTest;
	wbuf[4] = ioLevel;
    wbuf[5] = DstAddr >> 8;
    wbuf[6] = DstAddr;
	
//	set_temporary_DestAddr(DstAddr);
//	set_temporary_cast_mode(unicast);
//    usleep(100000);
	WriteComPort(wbuf, 7);
	
	printf("led value is : 0x%02x\r\n",ioLevel);
}

void testMotor(unsigned short DstAddr,unsigned char cmd)
{
	unsigned char wbuf[7];
	
	wbuf[0] = 'T';
	wbuf[1] = 'S';
	wbuf[2] = 'T';
	wbuf[3] = cmdMotorTest;
	wbuf[4] = cmd;
    wbuf[5] = DstAddr >> 8;
    wbuf[6] = DstAddr;
	
//	set_temporary_DestAddr(DstAddr);
//	set_temporary_cast_mode(unicast);
//    usleep(100000);
	WriteComPort(wbuf, 7);
	
	switch(cmd)
	{
		case cmdStop:
			printf("motor is stop...\r\n");
		break;
		case cmdForward:
			printf("motor is forwarding...\r\n");
		break;
		case cmdReverse:
			printf("motor is reversing...\r\n");
		break;
		default:
			printf("cmd is error...\r\n");
		break;
	}
}

void restoreFactoryConfig(unsigned short DstAddr)
{
	unsigned char wbuf[6];
	wbuf[0] = 'C';
	wbuf[1] = 'F';
	wbuf[2] = 'G';
 	wbuf[3] = cmdRestoreFactoryConfig;
	wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;

//	set_temporary_DestAddr(DstAddr);
//        usleep(100000);
	WriteComPort(wbuf, 6);
	printf("restore node 0x%04x factory config...\r\n",DstAddr);
}

void switchLockControl(unsigned short DstAddr,unsigned char cmd)
{
	unsigned char wbuf[7];
	wbuf[0] = 'C';
    wbuf[1] = 'T';
    wbuf[2] = 'L';
    wbuf[3] = 0x00;//cmd
    wbuf[4] = cmd;
    wbuf[5] = DstAddr >> 8;
    wbuf[6] = DstAddr;
        
//	set_temporary_cast_mode(unicast);
//	set_temporary_DestAddr(DstAddr);
//    usleep(100000);
	WriteComPort(wbuf, 7);
}

void ackNoControlCmd(unsigned short DstAddr)
{
    unsigned char wbuf[5];
    wbuf[0] = 'C';
    wbuf[1] = 'T';
    wbuf[2] = 'L';
    wbuf[3] = 0x01;//cmd
    wbuf[4] = 0x00;

//	set_temporary_cast_mode(unicast);
//	set_temporary_DestAddr(DstAddr);
//    usleep(100000);
	WriteComPort(wbuf, 5);
}
