#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "uart_raw.h"
#include "zlg_protocol.h"
#include "zlg_cmd.h"
#include "serial.h"

unsigned char iEEEAddress[8];

void communicate_thread(void)
{
	unsigned char rbuf[255];
	unsigned char rlen;

	while(1)
	{
		pthread_mutex_lock(&mut);
		rlen = ReadComPort(rbuf,100);
		pthread_mutex_unlock(&mut);
		if(rlen)
		{
			if(rbuf[0] == 'C' && rbuf[1] == 'F' && rbuf[2] == 'G')
			{
				switch(rbuf[3])
				{
					case cmdCheckIn:
						//iEEEAddress = (unsigned char *)malloc();
						memcpy(&iEEEAddress,&rbuf[4],8);
						ackRegisterNetwork(0x0001,Allow);
						printf("node is checking in...\r\n");
					break;
					case cmdAckChangeNodeType:						
						printf("change node type ACK...\r\n");
					break;
					case cmdAckLinkTest:
						printf("LinkTest ACK...\r\n");
					break;
					default:
					break;
				}
				
			}
			
			//printf("rbuf is:%s\r\n",rbuf);
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

void ackRegisterNetwork(unsigned short NetAddress,ackCmd_t cmd)
{
	char wbuf[15];
	char str[20];
	unsigned char *ieeeAddress = iEEEAddress;
 //       long long temp;
	wbuf[0] = 'C';
	wbuf[1] = 'F';
	wbuf[2] = 'G';	
	wbuf[3] = cmdAckCheckIn;	
	memcpy(&wbuf[4],(const char *)ieeeAddress,8);
	wbuf[12] = cmd;
	wbuf[13] = NetAddress >> 8;
	wbuf[14] = NetAddress;
	
	set_temporary_cast_mode(unicast);
	WriteComPort((unsigned char *)wbuf, 15);

//temp = *(long long*)iEEEAddress;
//printf("%016x\r\n",temp);
	mac2str(str,(const char *)iEEEAddress);
	if(cmd == Allow)
		printf("have ack 0x%s allocate local address is 0x%04x allow...\r\n",str,NetAddress);
	else
		printf("have ack 0x%s allocate local address is 0x%04x refuse...\r\n",str,NetAddress);
}

void changeNodeType(devTypeCmd_t deviceType)
{
	//int i;
	char wbuf[14];
	//char str1[10];
    	//char str2[10];
	unsigned char *ieeeAddress = iEEEAddress;
	wbuf[0] = 'C';
	wbuf[1] = 'F';
	wbuf[2] = 'G';	
	wbuf[3] = cmdChangeNodeType;
	memcpy(&wbuf[4],(const char *)ieeeAddress,8);
	wbuf[11] = deviceType;
	
	set_temporary_cast_mode(unicast);
	WriteComPort((unsigned char *)wbuf, 12);
	/*
	strcpy(str1,""); 
    for(i = 0;i < 8;i++)
    {
        sprintf(str2,"%02x",*((const char *)ieeeAddress)&0x0ff);
        strcat(str1,str2);
		ieeeAddress ++;
    }
	if(deviceType == endDevice)
		printf("have changed 0x%s deviceType : endDevice...\r\n",str1);
	else
		printf("have changed 0x%s deviceType : routerDevice...\r\n",str1);*/
}

void changePanidChannel(unsigned short panid,unsigned char channel)
{
	char wbuf[7];
	
	wbuf[0] = 'C';
	wbuf[1] = 'F';
	wbuf[2] = 'G';
	wbuf[3] = cmdChangePanidChannel;
	wbuf[4] = panid >> 8;
	wbuf[5] = panid;
	wbuf[6] = channel;
	
	set_temporary_cast_mode(broadcast);
	WriteComPort((unsigned char *)wbuf, 7);
	
	printf("have changed all node's panid to 0x%04x,channel to %d...\r\n",panid,channel);	
}

void resetAllNode(void)
{
	char wbuf[4];
	
	wbuf[0] = 'C';
	wbuf[1] = 'F';
	wbuf[2] = 'G';
	wbuf[3] = cmdAllNodeReset;
	
	set_temporary_cast_mode(broadcast);
	WriteComPort((unsigned char *)wbuf, 4);
	
	printf("have reset all node...\r\n");
}

void testLink(void)
{
	unsigned char wbuf[4];
	
	wbuf[0] = 'C';
	wbuf[1] = 'F';
	wbuf[2] = 'G';
	wbuf[3] = cmdLinkTest;
	
	set_temporary_cast_mode(unicast);
	WriteComPort( wbuf, 4);
	
	printf("taking link test...\r\n");
}

void startSensorCalibration(void)
{
	unsigned char wbuf[4];
	
	wbuf[0] = 'S';
	wbuf[1] = 'E';
	wbuf[2] = 'N';
	wbuf[3] = cmdSensorCal;
	
	set_temporary_cast_mode(broadcast);
	WriteComPort(wbuf, 4);
	
	printf("taking sensor calibration...\r\n"); 
}

void testBeep(unsigned short DstAddr,unsigned char cmd)
{
	unsigned char wbuf[5];
	
	wbuf[0] = 'T';
	wbuf[1] = 'S';
	wbuf[2] = 'T';
	wbuf[3] = cmdBeepTest;
	wbuf[4] = cmd;
	
	set_temporary_DestAddr(DstAddr);
	set_temporary_cast_mode(unicast);
	WriteComPort(wbuf, 5);
	
	if(cmd == cmdSilence)
		printf("node 0x%04x beep start to silence...\r\n",DstAddr);
	else
		printf("node 0x%04x beep start to buzzing...\r\n",DstAddr);
}

void testLed(unsigned short DstAddr,unsigned char ioLevel)
{
	unsigned char wbuf[5];
	
	wbuf[0] = 'T';
	wbuf[1] = 'S';
	wbuf[2] = 'T';
	wbuf[3] = cmdLedTest;
	wbuf[4] = ioLevel;
	
	set_temporary_DestAddr(DstAddr);
	set_temporary_cast_mode(unicast);
	WriteComPort(wbuf, 5);
	
	printf("led value is : 0x%02x\r\n",ioLevel);
}

void testMotor(unsigned short DstAddr,unsigned char cmd)
{
	unsigned char wbuf[5];
	
	wbuf[0] = 'T';
	wbuf[1] = 'S';
	wbuf[2] = 'T';
	wbuf[3] = cmdMotorTest;
	wbuf[4] = cmd;
	
	set_temporary_DestAddr(DstAddr);
	set_temporary_cast_mode(unicast);
	WriteComPort(wbuf, 5);
	
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
void switchLockControl(unsigned short DstAddr,unsigned char cmd)
{
	unsigned char wbuf[5];
	
	wbuf[0] = 'C';
	wbuf[1] = 'T';
	wbuf[2] = 'L';
	wbuf[3] = 0x00;//cmd
	wbuf[4] = cmd;

	set_temporary_DestAddr(DstAddr);
	set_temporary_cast_mode(unicast);
	WriteComPort(wbuf, 5);
}
