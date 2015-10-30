#include "uart_raw.h"
#include "zlg_cmd.h"
#include <string.h>
#include "gpio.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <asm/termios.h>
//#include <pthread.h>
#include "serial.h"

#define ttyO0  0
#define ttyO1  1
#define ttyO2  2
#define ttyO3  3
#define ttyO4  4
#define ttyO5  5

static unsigned char wbuf[100],rbuf[255];
const unsigned short broadcastAddr = 0xffff;

void write_local_cfg(void);

static int n_com_port = ttyO1;
dev_info_t stDevInfo;
dev_info_t remoteDevInfo;
search_info_t searchInfo;

void serialport_init(void)
{
        int ret = -1;

        ret = OpenComPort(n_com_port, 115200, 8, "1", 'N');
        if (ret < 0) {
                fprintf(stderr, "Error: Opening Com Port %d\n", n_com_port);
                return;
        }else{
                printf("Open Com Port %d Success, Now begin work\n", n_com_port);
        }
}

void init_zlg_zm516x(void)
{
    gpio_init();
    sleep_zm516x(1);
    serialport_init();
    usleep(100000);
    sleep_zm516x(0);
    read_local_cfg();
    write_local_cfg();    
	set_temporary_cast_mode(broadcast);
}

void read_local_cfg(void)
{
    int len = 0;
    
    char str1[33];
    char str2[10];

    int i;
    
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enReadLoacalCfg;
    wbuf[4] = wbuf[0] + wbuf[1] + wbuf[2] + wbuf[3];
    do{
        printf("get device info...\r\n");
        WriteComPort(wbuf, 5);
        usleep(100000);
        len = ReadComPort(rbuf,255);
        printf("device response data length is %d\r\n",len);
    }while(len < 74);
    
    memcpy(&stDevInfo,&rbuf[4],65);
    

    printf("--------------read begin-----------------\r\n");
    // display device info
    printf("Device name is:%s\r\n",stDevInfo.devName);
    printf("Device password is:%s\r\n",stDevInfo.devPwd);
    printf("Device type is:");
    switch(stDevInfo.devMode)
    {
        case 0:
        printf("End Device\r\n");
        break;
        case 1:
        printf("Router Device\r\n");
        break;
        default:
        printf("unknown\r\n");
        break;
    }
    printf("channel id is: %d\r\n",stDevInfo.devChannel);
    strcpy(str1,"");
    for(i = 0;i < 2;i++)
    {
        sprintf(str2,"%02x",stDevInfo.devPanid[i]&0x0ff);
        strcat(str1,str2);
    }
    printf("panid : 0x%s\r\n",str1);
    strcpy(str1,"");
    for(i = 0;i < 2;i++)
    {
        sprintf(str2,"%02x",stDevInfo.devLoacalNetAddr[i]&0x0ff);
        strcat(str1,str2);
    }
    printf("loacal net address is: 0x%s\r\n",str1);
    strcpy(str1,"");
    for(i = 0;i < 8;i++)
    {
        sprintf(str2,"%02x",stDevInfo.devLoacalIEEEAddr[i]&0x0ff);
        strcat(str1,str2);
    }
    printf("loacal IEEE address is: 0x%s\r\n",str1);
    strcpy(str1,"");
    for(i = 0;i < 2;i++)
    {
        sprintf(str2,"%02x",stDevInfo.devDestNetAddr[i]&0x0ff);
        strcat(str1,str2);
    }
    printf("dest net address is: 0x%s\r\n",str1);
    strcpy(str1,"");
    for(i = 0;i < 8;i++)
    {
        sprintf(str2,"%02x",stDevInfo.devDestIEEEAddr[i]&0x0ff);
        strcat(str1,str2);
    }
    printf("dest IEEE address is: 0x%s\r\n",str1);

    
    
    printf("Device power level is: %d\r\n",stDevInfo.devPowerLevel);
    printf("Retry num is: %d\r\n",stDevInfo.devRetryNum);
    printf("Retry time slot is: %d ms\r\n",stDevInfo.devTranTimeout * 10);

    printf("Device uart serial baudrate is :");
    switch(stDevInfo.devSerialRate)
    {
        case 1:
        printf("2400\r\n");
        break;
        case 2:
        printf("4800\r\n");
        break;
        case 3:
        printf("9600\r\n");
        break;
        case 4:
        printf("19200\r\n");
        break;
        case 5:
        printf("38400\r\n");
        break;
        case 6:
        printf("57600\r\n");
        break;
        case 7:
        printf("115200\r\n");
        break;
        default:
        printf("unknown\r\n");
        break;
    }
    printf("firmware version is:0x%04x\r\n",(unsigned short)rbuf[72] << 8 | rbuf[73]);
	printf("----------end-of-device-info--------------\r\n");
    //printf("write local cfg\r\n");
    //write_local_cfg();
}

int get_gateway_mac_addr(unsigned char *macAddr)
{
    int i;
    for(i = 0; i < 8; i++)
    {
      if(stDevInfo.devLoacalIEEEAddr[i] != 0)
        break;
    }
    if(i == 8)
      return -1;
    memcpy((void *)macAddr, (const void *)&stDevInfo.devLoacalIEEEAddr[0], 8);
    return 0;    
}

void write_local_cfg(void)
{
    int i,len = 0;
    unsigned char sum = 0;
    unsigned char buf[100],responsebuf[10];
    
    buf[0] = 0xab;
    buf[1] = 0xbc;
    buf[2] = 0xcd;
    buf[3] = enModifyCfg;
    buf[4] = stDevInfo.devLoacalNetAddr[0];
    buf[5] = stDevInfo.devLoacalNetAddr[1];
    memset(stDevInfo.devLoacalNetAddr,0,2);
    stDevInfo.devChannel = 15;
    stDevInfo.devPanid[0] = 0x00;
    stDevInfo.devPanid[1] = 0x00;
    stDevInfo.devDestNetAddr[0] = 0xFF;
    stDevInfo.devDestNetAddr[1] = 0xFE;
    memcpy(&buf[6],&stDevInfo,65);
    
    for(i = 0;i < (6 + 65);i++)
    {
        sum += buf[i];
    }
    buf[6 + 65] = sum;

    do
    {
	printf("start to write local cfg...\r\n");
        WriteComPort(buf, 6 + 65 + 1);
        usleep(10000);
        len = ReadComPort(responsebuf,10);
    }while(len != 7);
	
    usleep(100000);
    reset_zm516x();
    usleep(100000);
    printf("--------write local config success------\r\n");
}
/*
void send_data_to_remote_node(unsigned char *destAddr,unsigned char *data,int len)
{
    unsigned char responslen = 0;
    unsigned char buf[10],responsebuf[10];

    if(memcmp(destAddr,broadcastAddr,2) == 0)
    {
        buf[0] = 0xde;
        buf[1] = 0xdf;
        buf[2] = 0xef;
        buf[3] = enSetUnicastOrBroadcast;
        buf[4] = 0x01;
        printf("Send broadcast\r\n");
        while(responslen != 7)
        {
            WriteComPort(buf, 5);
            responslen = ReadComPort(responsebuf,10);
        }
    }
    else
    {
        buf[0] = 0xde;
        buf[1] = 0xdf;
        buf[2] = 0xef;
        buf[3] = enSetDestAddr;
        memcpy(&buf[4],destAddr,2);
        printf("Send unicast\r\n");
        while(responslen != 5)
        {
            WriteComPort(buf,6);
            usleep(10000);
            responslen = ReadComPort(responsebuf,10);
        }
    }
    WriteComPort(data, len);
    printf("Send over \r\n");
}
*/

void set_channel_nv(unsigned char nv)
{
    unsigned char i,sum = 0;  
    unsigned char rlen = 0;
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enSetChannelNv;
    wbuf[4] = nv;
    for(i = 0;i < 5;i++)
	sum += wbuf[i];
    wbuf[5] = sum;
    while(rlen != 5)
    {
		printf("start to set channel to %d...\r\n",nv);
    	WriteComPort(wbuf, 6);
                usleep(100000);
		rlen = ReadComPort(rbuf,10);
    }
    if(rbuf[0] == 0xab && rbuf[1] == 0xbc && rbuf[2] == 0xcd)
    {
		if(rbuf[3] == enSetChannelNv)
		{
			if(rbuf[4] == 0x00)
			{
				printf("set channel %d success!\r\n",nv);
				return;
			}
		}
    }
    printf("set channel %d failed!\r\n",nv);
}

void search_node(void)
{
    unsigned char i, sum = 0;
    unsigned char rlen = 0;
    //char inputbuf[2];
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enSearchNode;
    for(i = 0;i < 4; i++)
	sum += wbuf[i];
    wbuf[4] = sum;
    do
    {
		printf("start to search node...\r\n");
    	        WriteComPort(wbuf, 5);
		/*if(fgets(inputbuf,2,stdin))
		{
			inputbuf[strlen(inputbuf)] = '\0';
			if(!strncmp((const char *)inputbuf,"q",1) || !strncmp((const char *)inputbuf,"c",1))
		 		return;
		}*/
		usleep(10000);
		rlen = ReadComPort(rbuf,20);
    }while(rlen != 13);
    if(rbuf[0] == 0xab && rbuf[1] == 0xbc && rbuf[2] == 0xcd)
    { 
		if(rbuf[3] == enSearchNode)
		{
			memcpy(&searchInfo,&rbuf[4],sizeof(search_info_t));
		}
    }
	else
	{
		printf("search error!\r\n");
		return;
	}
    printf("--------Search Node Info is:--------------\r\n");
    printf("device type is:0x%02x%02x\r\n",searchInfo.deviceType[0],searchInfo.deviceType[1]);
    printf("channel is:%d\r\n",searchInfo.channel);
    printf("rate is:0x%02x\r\n",searchInfo.rate);
    printf("Netnum is:0x%02x%02x\r\n",searchInfo.netNum[0],searchInfo.netNum[1]);
    printf("loacal address is:0x%02x%02x\r\n",searchInfo.localAddr[0],searchInfo.localAddr[1]);
    printf("run status is:%x\r\n",searchInfo.runStatus);
    printf("--------end-of-search-node-info------------\r\n");
}

void get_remote_info(unsigned short DstAddr)
{
    unsigned char i,sum = 0;
    unsigned char rlen = 0;
	
    char str1[33];
    char str2[10];
	
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enGetRemoteInfo;
    wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;
    for(i = 0; i < 6; i++)
      sum += wbuf[i];
    wbuf[6] = sum;
    do{
		printf("start get remote node 0x%04x info...\r\n",DstAddr);
    		WriteComPort(wbuf, 7);
		usleep(100000);
		rlen = ReadComPort(rbuf,80);
		printf("response_len is:%d\r\n",rlen);
    }while(rlen < 74);
    if((rbuf[0] == 0xab)&&(rbuf[1] == 0xbc)&&(rbuf[2] == 0xcd))
    {
		if(rbuf[3] == enGetRemoteInfo)
		{
			memcpy(&remoteDevInfo,&rbuf[4],sizeof(dev_info_t));
		}
    }
    else
    {	
		printf("get info failed!\r\n");
		return;
    }
	
	
    memcpy(&remoteDevInfo,&rbuf[4],sizeof(dev_info_t));   
    printf("----remote device info is below:-----\r\n");
    printf("Device name is:%s\r\n",remoteDevInfo.devName);
    printf("Device password is:%s\r\n",remoteDevInfo.devPwd);
    printf("Device type is:");
    switch(remoteDevInfo.devMode)
    {
        case 0:
        printf("End Device\r\n");
        break;
        case 1:
        printf("Router Device\r\n");
        break;
        default:
        printf("unknown\r\n");
        break;
    }
    printf("channel id is: %d\r\n",remoteDevInfo.devChannel);
    strcpy(str1,"");
    for(i = 0;i < 2;i++)
    {
        sprintf(str2,"%02x",remoteDevInfo.devPanid[i]&0x0ff);
        strcat(str1,str2);
    }
    printf("panid is: 0x%s\r\n",str1);
    strcpy(str1,"");
    for(i = 0;i < 2;i++)
    {
        sprintf(str2,"%02x",remoteDevInfo.devLoacalNetAddr[i]&0x0ff);
        strcat(str1,str2);
    }
    printf("loacal net address is: 0x%s\r\n",str1);
    strcpy(str1,"");
    for(i = 0;i < 8;i++)
    {
        sprintf(str2,"%02x",remoteDevInfo.devLoacalIEEEAddr[i]&0x0ff);
        strcat(str1,str2);
    }
    printf("loacal IEEE address is: 0x%s\r\n",str1);
    strcpy(str1,"");
    for(i = 0;i < 2;i++)
    {
        sprintf(str2,"%02x",remoteDevInfo.devDestNetAddr[i]&0x0ff);
        strcat(str1,str2);
    }
    printf("dest net address is: 0x%s\r\n",str1);
    strcpy(str1,"");
    for(i = 0;i < 8;i++)
    {
        sprintf(str2,"%02x",remoteDevInfo.devDestIEEEAddr[i]&0x0ff);
        strcat(str1,str2);
    }
    printf("dest IEEE address is: 0x%s\r\n",str1);

    
    
    printf("Device power level is: %d\r\n",remoteDevInfo.devPowerLevel);
    printf("Retry num is: %d\r\n",remoteDevInfo.devRetryNum);
    printf("Retry time slot is: %d ms\r\n",remoteDevInfo.devTranTimeout * 10);

    printf("Device uart serial baudrate is :");
    switch(remoteDevInfo.devSerialRate)
    {
        case 1:
        printf("2400\r\n");
        break;
        case 2:
        printf("4800\r\n");
        break;
        case 3:
        printf("9600\r\n");
        break;
        case 4:
        printf("19200\r\n");
        break;
        case 5:
        printf("38400\r\n");
        break;
        case 6:
        printf("57600\r\n");
        break;
        case 7:
        printf("115200\r\n");
        break;
        default:
        printf("unknown\r\n");
        break;
    }
    printf("firmware version is:0x%04x\r\n",(unsigned short)rbuf[72] << 8 | rbuf[73]);
	printf("-------end-of-remote-device-info-------\r\n");
	
}

void write_remote_cfg(unsigned short DstAddr, dev_info_t *DevInfo )
{
    unsigned char i,sum = 0;
    unsigned char rlen = 0;
	
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enModifyCfg;
    wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;
    memcpy(&wbuf[6],DevInfo,65);
    for(i = 0;i < (6 + 65);i++)
    {
        sum += wbuf[i];
    }
    wbuf[6 + 65] = sum;
	
    do{
		printf("start to write remote node 0x%04x cfg...\r\n",DstAddr);
    		WriteComPort(wbuf, 72);
		usleep(100000);
		rlen = ReadComPort(rbuf,10);
    }while(rlen != 7);
    if((rbuf[0] == 0xab)&&(rbuf[1] == 0xbc)&&(rbuf[2] == 0xcd))
    {
		if(rbuf[3] == enModifyCfg)
		{
			if(rbuf[6] == 0x00)
			{
				printf("write 0x%02x%02x node cfg success!\r\n",rbuf[4],rbuf[5]);
				return;
			}
		}
    }
    printf("write 0x%04x node cfg failed!\r\n",DstAddr);
}

void reset_node(unsigned short DstAddr)
{
    unsigned char i, sum = 0;
	
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enResetNode;
    wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;
    wbuf[6] = 0x00;
    wbuf[7] = 0x01;
    for(i = 0; i < 8; i++)
      sum += wbuf[i];
    wbuf[8] = sum;
    
    WriteComPort(wbuf, 9);
	printf("have reset node 0x%04x\r\n",DstAddr);
}

void restore_factory_settings(unsigned short DstAddr)
{
    unsigned char i, sum = 0;
	
    wbuf[0] = 0xab;
    wbuf[1] = 0xbc;
    wbuf[2] = 0xcd;
    wbuf[3] = enResetCfg;
    wbuf[4] = DstAddr >> 8;
    wbuf[5] = DstAddr;
    wbuf[6] = 0x00;
    wbuf[7] = 0x01;
    for(i = 0; i < 8; i++)
      sum += wbuf[i];
    wbuf[8] = sum;
    
    WriteComPort(wbuf, 9);
	printf("have restore 0x%04x node factory settings\r\n",DstAddr);
}

void set_temporary_channel(unsigned char channel)
{
	unsigned char rlen = 0;
	wbuf[0] = 0xde;
	wbuf[1] = 0xdf;
	wbuf[2] = 0xef;  
	wbuf[3] = enSetChannel;  
	wbuf[4] = channel;
	do
	{
		printf("start to set temp channel to %d\r\n",channel);
		WriteComPort(wbuf, 5);
                usleep(100000);
		rlen = ReadComPort(rbuf,10);
	}while(rlen != 5);
	if((rbuf[0] == 0xde)&&(rbuf[1] == 0xdf)&&(rbuf[2] == 0xef))
	{
		if(rbuf[3] == enSetChannel)
		{
			if(rbuf[4] == 0x00)
			{
				printf("set temp channel success...\r\n");
				return;
			}
		}
	}
	printf("set temp channel error...\r\n"); 
}

void set_temporary_DestAddr(unsigned short DestAddr)
{
	unsigned char rlen = 0;
	static unsigned char lastDestAddr = 0x00;
	
	if( DestAddr == lastDestAddr )
		return;
	else
		lastDestAddr = DestAddr;
	wbuf[0] = 0xde;
	wbuf[1] = 0xdf;
	wbuf[2] = 0xef;  
	wbuf[3] = enSetDestAddr; 
	wbuf[4] = DestAddr >> 8;
	wbuf[5] = DestAddr;

	printf("start to set DestAddr to 0x%04x\r\n",DestAddr);
	WriteComPort(wbuf, 6);
//	do
//	{
                usleep(100000);
		rlen = ReadComPort(rbuf,10);
/*	}while(rlen <= 5);
	if((rbuf[0] == 0xde)&&(rbuf[1] == 0xdf)&&(rbuf[2] == 0xef))
	{
		if(rbuf[3] == enSetDestAddr)
		{
			if(rbuf[4] == 0x00)
			{
				return;
			}
			else
				printf("set DestAddr failed...\r\n");
		}
	}
	printf("set DestAddr error...\r\n");*/ 
}

void set_temporary_ShowSrcAddr(showSrcAddr_cmd_t cmd)
{
	unsigned char rlen = 0;
	
	wbuf[0] = 0xde;
	wbuf[1] = 0xdf;
	wbuf[2] = 0xef;
	wbuf[3] = enShowSrcAddr;  
	wbuf[4] = cmd;

		switch(cmd)
		{
			case show_enable:
				printf("start to set show src address\r\n");
				break;
			case show_disable:
			    printf("start to set not show src address\r\n");
				break;
			default:
			break;
		}
		WriteComPort(wbuf, 5);
//	do
//	{
                usleep(100000);
		rlen = ReadComPort(rbuf,10);
/*	}while(rlen <= 5);
	if((rbuf[0] == 0xde)&&(rbuf[1] == 0xdf)&&(rbuf[2] == 0xef))
	{
		if(rbuf[3] == enShowSrcAddr)
		{
			if(rbuf[4] == 0x00)
			{
				printf("set ShowSrcAddr success...\r\n");
				return;
			}
		}
	}
	printf("set ShowSrcAddr error...\r\n");*/ 
}

void set_temporary_io_dir(unsigned short DstAddr,unsigned char IO_Dir)
{
	unsigned char rlen = 0;
	
	wbuf[0] = 0xde;
	wbuf[1] = 0xdf;
	wbuf[2] = 0xef; 
	wbuf[3] = enSetIoDirection;
	wbuf[4] = DstAddr >> 8;
	wbuf[5] = DstAddr;
	wbuf[6] = IO_Dir;

	do
	{
		printf("start to set 0x%04x node's io_dir:0x%02x\r\n",DstAddr,IO_Dir);
		WriteComPort(wbuf, 7);


                usleep(100000);
		rlen = ReadComPort(rbuf,10);
	}while(rlen != 7);
	if((rbuf[0] == 0xde)&&(rbuf[1] == 0xdf)&&(rbuf[2] == 0xef))
	{
		if(rbuf[3] == enSetIoDirection)
		{
			if(rbuf[6] == 0x00)
			{
				printf("set node 0x%02x%02x io_dir success...\r\n",rbuf[4],rbuf[5]);
				return;
			}
		}
	}
	printf("set node's io_dir error...\r\n"); 
}

unsigned char read_temporary_io_level(unsigned short DstAddr)
{
	unsigned char rlen = 0;
	
	wbuf[0] = 0xde;
	wbuf[1] = 0xdf;
	wbuf[2] = 0xef;
	wbuf[3] = enReadIoStatus;
	wbuf[4] = DstAddr >> 8;
	wbuf[5] = DstAddr;

	do
	{
		printf("start to read 0x%04x node's io_level...\r\n",DstAddr);
		WriteComPort(wbuf, 6);
                usleep(100000);
		rlen = ReadComPort(rbuf,10);
	}while(rlen != 7);
	if((rbuf[0] == 0xde)&&(rbuf[1] == 0xdf)&&(rbuf[2] == 0xef))
	{
		if(rbuf[3] == enReadIoStatus)
		{
			printf("read node 0x%02x%02x io_level is:0x%02x\r\n",rbuf[4],rbuf[5],rbuf[6]);
			return rbuf[6];
		}
	}
	printf("read node's io_level error...\r\n");
	return 0;
}

void set_temporary_io_level(unsigned short DstAddr,unsigned char IO_Level)
{
	unsigned char rlen = 0;
	
	wbuf[0] = 0xde;
	wbuf[1] = 0xdf;
	wbuf[2] = 0xef;
	wbuf[3] = enSetIoStatus;
	wbuf[4] = DstAddr >> 8;
	wbuf[5] = DstAddr;
	wbuf[6] = IO_Level;

	do
	{
		printf("start to set 0x%04x node's io_level to 0x%02x\r\n",DstAddr,IO_Level);
		WriteComPort(wbuf, 7);
                usleep(100000);
		rlen = ReadComPort(rbuf,10);
	}while(rlen != 7);
	if((rbuf[0] == 0xde)&&(rbuf[1] == 0xdf)&&(rbuf[2] == 0xef))
	{
		if(rbuf[3] == enSetIoStatus)
		{
			if(rbuf[6] == 0x00)
			{
				printf("set node 0x%02x%02x io_level to 0x%02x success...\r\n",rbuf[4],rbuf[5],IO_Level);
				return;
			}
		}
	}
	printf("set node's io_level error...\r\n");
}

unsigned short read_temporary_adc_value(unsigned short DstAddr)
{
	unsigned char rlen = 0;
	
	wbuf[0] = 0xde;
	wbuf[1] = 0xdf;
	wbuf[2] = 0xef;
	wbuf[3] = enReadAdcValue;
	wbuf[4] = DstAddr >> 8;
	wbuf[5] = DstAddr;
	wbuf[6] = 0x01;
	do
	{
		printf("start to read 0x%04x node's adc_value...\r\n",DstAddr);
		WriteComPort(wbuf, 7);
                usleep(100000);
		rlen = ReadComPort(rbuf,10);
	}while(rlen != 8);
	if((rbuf[0] == 0xde)&&(rbuf[1] == 0xdf)&&(rbuf[2] == 0xef))
	{
		if(rbuf[3] == enReadAdcValue)
		{
			printf("read node 0x%02x%02x adc_value is 0x%02x%02x\r\n",rbuf[4],rbuf[5],rbuf[6],rbuf[7]);
			return ((rbuf[6] << 8) | rbuf[7]);
		}
	}
	printf("read node's adc_value error...\r\n");
	return 0;
}

void enter_sleep_mode(void)
{
	wbuf[0] = 0xde;
	wbuf[1] = 0xdf;
	wbuf[2] = 0xef;
	wbuf[3] = enEnterSleepMode;
	wbuf[4] = 0x01;

	WriteComPort(wbuf, 5);
}

void set_temporary_cast_mode(cast_mode_t mode)
{
	unsigned char rlen = 0;
	static unsigned char mode_last = 0xff;

	if( mode == mode_last ) 
		return;
	else
		mode_last = mode;	
	wbuf[0] = 0xde;
	wbuf[1] = 0xdf;
	wbuf[2] = 0xef;
	wbuf[3] = enSetUnicastOrBroadcast;
	wbuf[4] = mode;
//	do
//	{
		switch(mode)
		{
			case unicast:
				printf("####start to set cast mode to unicast mode...\r\n");
				break;
			case broadcast:
				printf("####start to set cast mode to broadcast mode...\r\n");
				break;
			default:
				break;
		}
		WriteComPort(wbuf, 5);
        usleep(1000000);
		rlen = ReadComPort(rbuf,255);
/*	}while(rlen <= 5);
	if((rbuf[0] == 0xde)&&(rbuf[1] == 0xdf)&&(rbuf[2] == 0xef))
	{
		if(rbuf[3] == enSetUnicastOrBroadcast)
		{
			if(rbuf[4] == 0x00)
			{
				return;
			}
			else
				printf("set cast mode failed...\r\n");
		}
	}
	printf("set cast mode error...\r\n");*/
}

unsigned char read_temporary_node_rssi(unsigned short DstAddr)
{
	unsigned char rlen = 0;
	
	wbuf[0] = 0xde;
	wbuf[1] = 0xdf;
	wbuf[2] = 0xef;
	wbuf[3] = enReadNodeRssi; 
	wbuf[4] = DstAddr >> 8;
	wbuf[5] = DstAddr;
	do
	{
		printf("start to read 0x%04x node's rssi...\r\n",DstAddr);
		WriteComPort(wbuf, 6);
		usleep(100000);
		rlen = ReadComPort(rbuf,10);
	}while(rlen != 5);
	if((rbuf[0] == 0xde)&&(rbuf[1] == 0xdf)&&(rbuf[2] == 0xef))
	{
		if(rbuf[3] == enReadNodeRssi)
		{
			printf("read node 0x%04x rssi is 0x%02x.\r\n",DstAddr,rbuf[4]);
			return rbuf[4];
		}
	}
	printf("read node's rssi error...\r\n");
	return 0;
}

void send_data_to_remote_node(unsigned short destAddr,unsigned char *data,int len)
{
    if(destAddr == broadcastAddr)
    {
		set_temporary_cast_mode(broadcast);
    }
    else
    {
		set_temporary_DestAddr(destAddr);
    }
	
    WriteComPort(data, len);
    printf("Send over \r\n");
}
