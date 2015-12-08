/*****************************************************************************************************
**name	xbee_routine.c
**brief	主要包括硬件的初始化，通讯协议线程的建立及初始化，读取/写入串口数据
**
*****************************************************************************************************/

#include "xbee_routine.h"
#include "xbee_api.h"
#include "serial.h"
#include "sys/queue.h"
#include <time.h>
#include "serial.h"
#include "string.h"
#include "gpio.h"
/**************************gloable variable************************/
CircularQueueType serial_rbuf;			//the serial read buffer
#if 0//__XBEE_TEST_LAR_NODE__
SourceRouterLinkType *pLinkHead=NULL;
#endif
uint8 send_xbee_state=0;
uint32 waite_send_head_num=0;
//uint32 send_data_timeout=0;
CircularQueueType trans_status_buf;		//xbee transmit request API buffer
CircularQueueType xbee_other_api_buf;			//
CircularQueueType serial_wbuf;			//the serial write buffer
CircularQueueType trans_req_buf;		//xbee transmit status API buffer
CircularQueueType route_record_buf;		//
SourceRouterLinkType *pSourcePathList=NULL;
#if 0//__XBEE_TEST_LAR_NODE__
CircularQueueType ts_buf;
#endif
/*************************** mutex ********************************/
pthread_mutex_t mutex01_serial_rbuf = PTHREAD_MUTEX_INITIALIZER;
#if 0//__XBEE_TEST_LAR_NODE__
pthread_mutex_t mutex02_pLinkHead = PTHREAD_MUTEX_INITIALIZER;
#endif
pthread_mutex_t mutex03_send_xbee_state = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex04_waite_send_head_num = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex05_send_data_timeout = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex06_waite_send_head = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex07_CoorInfo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex08_trans_status_buf = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex09_xbee_other_api_buf = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex10_serial_wbuf = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex11_route_record_buf = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex12_trans_req_buf = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex13_pSourcePathList = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex14_CoorInfo = PTHREAD_MUTEX_INITIALIZER;
#if 0//__XBEE_TEST_LAR_NODE__
pthread_mutex_t mutex14_ts_buf = PTHREAD_MUTEX_INITIALIZER;
#endif
/*************************** cond *********************************/
pthread_cond_t cond_send_xbee=PTHREAD_COND_INITIALIZER;

void xbee_routine_thread(void)
{
	int ret=0;
    pthread_t id;
	uint8 _i,_adr[8];
	uint8 *HeadMidAdr=NULL;

	time_t timetTime;
    struct tm *pTmTime;

	timetTime = time(NULL);		//获取当前系统时间
	//printf("timetTime=%d\n", (uint32)timetTime);
	strncpy(CoorInfo.logname,"xbee_dev_log_",13);
    pTmTime = localtime(&timetTime);	//time_t 结构转换成tm结构
    snprintf(CoorInfo.logname+13 , 24 , "%04d_%02d_%02d_%02d_%02d_%02d.txt", pTmTime->tm_year+1900, pTmTime->tm_mon+1, pTmTime->tm_mday, pTmTime->tm_hour, pTmTime->tm_min, pTmTime->tm_sec);
	printf("CoorInfo.logname=%s\n", CoorInfo.logname);
	err_log(CoorInfo.logname , strlen(CoorInfo.logname));

	xbee_gpio_init();
	//xbee_serial_port_init();

	for(_i=0;_i<8;_i++)
		_adr[_i] = 0;

	do{
		ret = pthread_mutex_lock(&mutex13_pSourcePathList);
		if(ret == 0)
			pSourcePathList = CreatRouterLink(_adr,0,HeadMidAdr,0);
		ret = pthread_mutex_unlock(&mutex13_pSourcePathList);
	}while(ret != 0);

	do{
		ret = pthread_mutex_lock(&mutex01_serial_rbuf);
		if(ret == 0)
			creat_circular_queue( &serial_rbuf );
		ret = pthread_mutex_unlock(&mutex01_serial_rbuf);
	}while(ret != 0);

	do{
		ret = pthread_mutex_lock(&mutex08_trans_status_buf);
		if(ret == 0)
			creat_circular_queue( &trans_status_buf );
		ret = pthread_mutex_unlock(&mutex08_trans_status_buf);
	}while(ret != 0);

	do{
		ret = pthread_mutex_lock(&mutex09_xbee_other_api_buf);
		if(ret == 0)
			creat_circular_queue( &xbee_other_api_buf );
		ret = pthread_mutex_unlock(&mutex09_xbee_other_api_buf);
	}while(ret != 0);

	do{
		ret = pthread_mutex_lock(&mutex10_serial_wbuf);
		if(ret == 0)
			creat_circular_queue( &serial_wbuf );
		ret = pthread_mutex_unlock(&mutex10_serial_wbuf);
	}while(ret != 0);

	do{
		ret = pthread_mutex_lock(&mutex12_trans_req_buf);
		if(ret == 0)
			creat_circular_queue( &trans_req_buf );
		ret = pthread_mutex_unlock(&mutex12_trans_req_buf);
	}while(ret != 0);

	do{
		ret = pthread_mutex_lock(&mutex11_route_record_buf);
		if(ret == 0)
			creat_circular_queue( &route_record_buf );
		ret = pthread_mutex_unlock(&mutex11_route_record_buf);
	}while(ret != 0);

	xbee_reset();
	sleep(1);

	//printf("\033[33mstart xbee_routine_thread_write_serial...\033[0m\r\n");
	do{
	    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_write_serial,NULL);
	    if(ret!=0)
		{
	        printf ("Create xbee_routine_thread_write_serial error!\n");
			err_log("Create xbee_routine_thread_write_serial error!" , strlen("Create xbee_routine_thread_write_serial error!"));
	    }
	}while(ret != 0);

	//printf("\033[33mstart xbee_routine_thread_read_serial...\033[0m\r\n");
	do{
	    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_read_serial,NULL);
	    if(ret!=0)
		{
	        printf ("Create xbee_routine_thread_read_serial error!\n");
			err_log("Create xbee_routine_thread_read_serial error!" , strlen("Create xbee_routine_thread_read_serial error!"));
	    }
	}while(ret != 0);

	//printf("\033[33mstart xbee_routine_thread_process_serial_rbuf...\033[0m\r\n");
	do{
    	ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_process_serial_rbuf,NULL);
    	if(ret!=0)
		{
       		printf ("Create xbee_routine_thread_process_serial_rbuf error!\n");
			err_log("Create xbee_routine_thread_process_serial_rbuf error!" , strlen("Create xbee_routine_thread_process_serial_rbuf error!"));
    	}
	}while(ret != 0);

	XBeeNetInit();
	get_mac();

	//printf("\033[33mstart xbee_routine_thread_process_trans_status_buf...\033[0m\r\n");
	do{
    	ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_process_trans_status_buf,NULL);
    	if(ret!=0)
		{
        	printf ("Create xbee_routine_thread_process_trans_status_buf error!\n");
			err_log("Create xbee_routine_thread_process_trans_status_buf error!" , strlen("Create xbee_routine_thread_process_trans_status_buf error!"));
    	}
	}
	while(ret != 0);

	//printf("\033[33mstart xbee_routine_thread_process_trans_req_buf...\033[0m\r\n");
	do{
		ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_process_trans_req_buf,NULL);
    	if(ret!=0)
		{
    	    printf ("Create xbee_routine_thread_process_trans_req_buf error!\n");
			err_log("Create xbee_routine_thread_process_trans_req_buf error!" , strlen("Create xbee_routine_thread_process_trans_req_buf error!"));
    	}
	}
	while(ret != 0);

	//printf("\033[33mstart xbee_routine_thread_process_route_record_buf...\033[0m\r\n");
	do{
		ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_process_route_record_buf,NULL);
    	if(ret!=0)
		{
			printf ("Create xbee_routine_thread_process_route_record_buf error!\n");
			err_log("Create xbee_routine_thread_process_route_record_buf error!" , strlen("Create xbee_routine_thread_process_route_record_buf error!"));
		}
	}
	while(ret != 0);

	//printf("\033[33mstart xbee_routine_thread_process_other_api_buf...\033[0m\r\n");
	do{
    	ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_process_other_api_buf,NULL);
    	if(ret!=0)
		{
			printf ("Create xbee_routine_thread_process_other_api_buf error!\n");
			err_log("Create xbee_routine_thread_process_other_api_buf error!" , strlen("Create xbee_routine_thread_process_other_api_buf error!"));
    	}
	}while(ret != 0);

#if __XBEE_TEST__
	printf("\033[33mstart xbee_routine_thread_test...\033[0m\r\n");
	do{
    	ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_test,NULL);
		if(ret!=0)
		{
	        printf ("Create xbee_routine_thread_test error!n");
	    }
	}while(ret != 0);
#endif

	//CoorInfo.net_state = 'o';
	while(1)
	{
		uint16 i=0,n=0;
		SourceRouterLinkType *p=NULL;
		ret = pthread_mutex_lock(&mutex13_pSourcePathList);
		if(ret == 0)
		{
			n = LinkLenth(pSourcePathList);
			for(i=1;i<=n;i++)
			{
				p = FindnNode(pSourcePathList,i);
				if(p->cnt-- < 1)
					DeleteNode(pSourcePathList,p);
				p = NULL;
			}
		}
		ret = pthread_mutex_unlock(&mutex13_pSourcePathList);
		usleep(5000000);
		//usleep(2000000);
	}
}

void xbee_routine_thread_process_other_api_buf(void)
{
	static uint8 rbuf[255]={0};
	uint16 len=0;
	int ret=0;
	while(1)
	{
		ret = pthread_mutex_lock(&mutex09_xbee_other_api_buf);
		if(ret == 0)
			len = read_one_package_f_queue( &xbee_other_api_buf , rbuf );
		ret = pthread_mutex_unlock(&mutex09_xbee_other_api_buf);
		if(len)
	 	{
			switch(rbuf[3])
			{
				case receive_packet:
					if(rbuf[15]=='C' && rbuf[16]=='F' && rbuf[17]=='G')
					{
						//TestPrintf("C F G",len,rbuf);
						XBeeProcessCFG(rbuf);
					}
					else if(rbuf[15]=='C' && rbuf[16]=='T' && rbuf[17]=='L')
						XBeeProcessCTL(rbuf);
					else if(rbuf[15]=='S' && rbuf[16]=='E' && rbuf[17]=='N')
					{
						//TestPrintf("S E N",len,rbuf);
						XBeeProcessSEN(rbuf);
					}
					else if(rbuf[15]=='O' && rbuf[16]=='T' && rbuf[17]=='A')
					{}
					break;
				case at_command_response:
					ProcessATRes(rbuf);
					break;
				case remoto_AT_command_response:
					if(*(rbuf+17) == 1 || *(rbuf+17) == 4)
					{
						//XBeeRemoteATCmd(rbuf+5 , rbuf+13 , 2 , (uint8*)(rbuf+15) , param , 1 , 1);
					}
					TestPrintf("remote",len,rbuf);
					break;
				default:
					break;
			}
		}
		usleep(15000);
	 }
}

void xbee_routine_thread_process_trans_status_buf(void)
{
	static uint8 rbuf[255]={0};
	uint16 len=0;
	int ret=0;
	while(1)
	{
		ret = pthread_mutex_lock(&mutex08_trans_status_buf);
		if(ret == 0)
			len = read_one_package_f_queue( &trans_status_buf , rbuf );
		ret = pthread_mutex_unlock(&mutex08_trans_status_buf);
		if(len)
		{
#if 0
			if(*(rbuf+8) != 0 ) {for(i=0;i<11;i++)	printf("%02x ",*(rbuf+i));	puts(" ");}
#endif
			ret = pthread_mutex_lock(&mutex03_send_xbee_state);
			if(ret ==0 && send_xbee_state > 0)
				send_xbee_state--;
			ret = pthread_cond_signal(&cond_send_xbee);
			pthread_mutex_unlock(&mutex03_send_xbee_state);
		}
		usleep(15000);
	}
}

void xbee_routine_thread_write_serial(void)
{
	static uint8 buf[255]={0};
	uint16 len=0;
	int ret=0;
	uint32 sleep_time;
	int8 bufs[1];
	bool state;
	while(1)
	{
		//printf("file : %s,line = %d\r\n",__FILE__,__LINE__);
		sleep_time = 15000;
		state = read_xbee_cts(bufs);
		if(state == true && *bufs == '1')
		{
			ret = pthread_mutex_lock(&mutex10_serial_wbuf);
			if(ret == 0)
				len = read_one_package_f_queue(&serial_wbuf , buf);
			ret = pthread_mutex_unlock(&mutex10_serial_wbuf);
			if(len)
			{
#if 0
				uint8 i=0;	printf("write serial : ");	for(i=0;i<len;i++)	printf("%02x ",buf[i]);	printf("\n");
#endif
				WriteComPort(buf, len);
			}
		}
		else
		{
			sleep_time = 1000000;
			puts("\033[34mwarring : xbee serial buffer is full!\033[0m");
		}
		usleep(sleep_time);
	}

}

void xbee_routine_thread_read_serial(void)
{
	uint32 len=0;
	int ret=0;
	static uint8 buf[255];
	while(1)
	{
		//printf("file : %s,line = %d\r\n",__FILE__,__LINE__);
		len = ReadComPort (buf, 255);
		//printf("file : %s,line = %d\r\n",__FILE__,__LINE__);
		if(len > 0)
		{
#if 0
			uint8 i=0;	printf("read serial : ");	for(i=0;i<len;i++)		printf("%02x " , *(buf+i));	puts(" ");
#endif
			//printf("file : %s,line = %d\r\n",__FILE__,__LINE__);
			ret = pthread_mutex_lock(&mutex01_serial_rbuf);
			if(ret == 0)
				write_cqueue( &serial_rbuf , buf , len );
			ret = pthread_mutex_unlock(&mutex01_serial_rbuf);
		}
		usleep(20000);
	}
}

void xbee_routine_thread_process_serial_rbuf(void)
{
	uint16 len=0;
	uint8 buf[255];
	int ret=0;
	while(1)
	{
		//printf("file : %s,line = %d\r\n",__FILE__,__LINE__);
		ret = pthread_mutex_lock(&mutex01_serial_rbuf);
		if(ret == 0)
			len = read_one_package_f_queue(&serial_rbuf , buf);
		ret = pthread_mutex_unlock(&mutex01_serial_rbuf);
#if 0
		uint8 i=0;	for(i=0;i<len;i++)	printf("\033[32m%02x \033[0m",*(buf+i));	printf("\033[31mlen = %d\033[0m\n",len);
#endif
		if(len)
		{
			switch(*(buf+3))
			{
				case transmit_status:
					ret = pthread_mutex_lock( &mutex08_trans_status_buf );
					if(ret == 0)
						write_cqueue( &trans_status_buf , buf , len );
					ret = pthread_mutex_unlock( &mutex08_trans_status_buf );
					break;
				case route_record_indicator:
					ret = pthread_mutex_lock( &mutex11_route_record_buf );
					if(ret == 0)
						write_cqueue( &route_record_buf , buf , len );
					ret = pthread_mutex_unlock( &mutex11_route_record_buf );
					break;
				default:
					ret = pthread_mutex_lock(&mutex09_xbee_other_api_buf);
					if(ret == 0)
						write_cqueue( &xbee_other_api_buf , buf , len );
					ret = pthread_mutex_unlock(&mutex09_xbee_other_api_buf);
					break;
			}
		}
		usleep(15000);
	}
}

void xbee_routine_thread_process_trans_req_buf(void)
{
	uint16 len=0;
	static uint8 rbuf[255];
	static uint8 tar[]={0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xfe};
	uint32 sleep_time;
	int ret=0;
	while(1)
	{
		sleep_time = 80000;
		ret = pthread_mutex_lock(&mutex12_trans_req_buf);
		if(ret == 0)
			len = read_one_package_f_queue(&trans_req_buf , rbuf);
		ret = pthread_mutex_unlock(&mutex12_trans_req_buf);
		if(len)
		{
#if 0
			uint16 i=0;	for(i=0;i<len;i++)	printf("%02x ",*(rbuf+i));	puts(" ");
#endif
			sleep_time = 15000;
			ret = pthread_mutex_lock(&mutex10_serial_wbuf);
			if(ret == 0)
				write_cqueue(&serial_wbuf , rbuf , len);
			ret = pthread_mutex_unlock(&mutex10_serial_wbuf);
			if(*(rbuf + 3) == 0x10 && arrncmp(tar,(rbuf+5),8) != 0)
			{
#if 0
				uint16 i=0;	for(i=0;i<len;i++)	printf("%02x ",*(rbuf+i));	puts(" ");
#endif
				ret = pthread_mutex_lock(&mutex03_send_xbee_state);
				if(ret == 0)
					send_xbee_state++;
				ret = pthread_mutex_unlock(&mutex03_send_xbee_state);
				sleep_time = 80000;
			}
		}
		usleep(sleep_time);
	}
}

void xbee_routine_thread_process_route_record_buf(void)
{
	uint8 buf[128];
	uint16 len=0;
	SourceRouterLinkType *p;
	int ret=0;
	while(1)
	{
		ret = pthread_mutex_lock(&mutex11_route_record_buf);
		if(ret == 0)
			len = read_one_package_f_queue(&route_record_buf , buf);
		ret = pthread_mutex_unlock(&mutex11_route_record_buf);
		if(len)
		{
#if 0
			uint8 i=0;	for(i=0;i<len;i++)	printf("%02x ",*(buf+i));	puts("\033[31m%\%\033[0m");
#endif
			ret = pthread_mutex_lock(&mutex13_pSourcePathList);
			if(ret == 0)
			{
				p = XBeeProcessRoutRcord(pSourcePathList , buf);
				p->cnt = 10;
			}
			ret = pthread_mutex_unlock(&mutex13_pSourcePathList);
		}
		usleep(50000);
	}
}

#if __XBEE_TEST__
void xbee_routine_thread_test(void)
{
	int8 in_cmd[100];
	int reval,ret=0;
	uint8 i;

	while(1)
	{
		reval = scanf("%s",in_cmd);
		if(strncmp("list",in_cmd,strlen(in_cmd)) == 0)
		{
			ret = pthread_mutex_lock(&mutex13_pSourcePathList);
			if(ret == 0)
				LinkPrintf(pSourcePathList);
			ret = pthread_mutex_unlock(&mutex13_pSourcePathList);
		}
		else if(strncmp("ar",in_cmd,strlen(in_cmd)) == 0)
		{
			XBeeSetAR(1,RES);
			puts("ar 20s");
		}
#if __XBEE_TEST_LAR_NODE__
		else if(strncmp("start",in_cmd,strlen("start")) == 0)
		{
			start = *(in_cmd+5);
			printf("start = %c\n",start);
		}
#endif
		else if(strncmp("ar0",in_cmd,strlen(in_cmd)) == 0)
		{
			XBeeSetAR(0,RES);
			puts("ar one times");
		}
		else if(strncmp("arclose",in_cmd,strlen(in_cmd)) == 0)
		{
			XBeeSetAR(0xff,RES);
			puts("ar has closed");
		}
		else if(strncmp("clear",in_cmd,strlen(in_cmd)) == 0)
		{
			ret = pthread_mutex_lock(&mutex03_send_xbee_state);
			if(ret == 0)
			{
				send_xbee_state=0;
				printf("\033[32msend_xbee_state = %d\033[0m\n",send_xbee_state);
			}
			ret = pthread_mutex_unlock(&mutex03_send_xbee_state);
		}
		else if(strncmp("op",in_cmd,strlen(in_cmd)) == 0)
		{
			XBeeReadAT("OP");
			sleep(1);
			printf("\033[35m64位panID:\033[0m");
			for(i=0;i<8;i++)
				printf("0x%02x ",CoorInfo.panID64[i]);
			printf("\n");
		}
		else if(strncmp("id",in_cmd,strlen(in_cmd)) == 0)
		{
			XBeeReadAT("ID");
			sleep(1);
			printf("\033[35m64位panID:\033[0m");
			for(i=0;i<8;i++)
				printf("0x%02x ",CoorInfo.panID64[i]);
			printf("\n");
		}
		else if(strncmp("oi",in_cmd,strlen(in_cmd)) == 0)
		{
			XBeeReadAT("OI");
			sleep(1);
			printf("\033[35m16位panID: \033[0m0x%04x\n",CoorInfo.panID16);
		}
		else if(strncmp("nj",in_cmd,strlen(in_cmd)) == 0)
		{
			XBeeReadAT("NJ");
			sleep(1);
			printf("\033[35m允许入网时间 \033[0m0x%02x\n",CoorInfo.nj);
		}
		else if(strncmp("ch",in_cmd,strlen(in_cmd)) == 0)
		{
			XBeeReadAT("CH");
			sleep(1);
			printf("\033[35m网络信道: \033[0m0x%04x\n",CoorInfo.channel);
		}
		else if(strncmp("readar",in_cmd,strlen(in_cmd)) == 0)
		{
			XBeeReadAT("AR");
			sleep(1);
			printf("\033[35mAR : \033[0m0x%02x\n",CoorInfo.ar);
		}
		else if(strncmp("check",in_cmd,strlen(in_cmd)) == 0)
		{
			if(networking_over() == 1)
				printf("\033[35m全部锁已入网\033[0m\n");
			else if(networking_over() == 0)
				printf("\033[35m有锁未入网\033[0m\n");
		}
		else
			printf("\033[35m无效的命令\033[0m\n");
		usleep(1000);
	}
}
#endif
void TestPrintf(int8* sss,int16 lens,uint8 *buf)
{
	int loop=0;

	printf("\033[34m数据序列--%s:数据长度--%d; 数据内容:\033[0m\n",sss,lens);
	for( loop = 0;loop < lens;loop ++)
	{
		switch(*(buf+3))
		{
			case 0x88:
				if(loop==5 || loop==6)
					printf("\033[32m%c \033[0m",(int8)buf[loop]);
				else
					printf("0x%02x ",buf[loop]);
				break;
			case 0x90:
				if(loop==15 || loop==16 || loop==17)
					printf("\033[32m%c \033[0m",(int8)buf[loop]);
				else
					printf("0x%02x ",buf[loop]);
				break;
			case 0xa1:
				if(loop == 3)
					printf("\033[32m0x%02x \033[0m",buf[loop]);
				else
					printf("0x%02x ",buf[loop]);
				break;
			case 0x8b:
				if(loop == 3)
					printf("\033[35m0x%02x \033[0m",buf[loop]);
				else
					printf("0x%02x ",buf[loop]);
				break;
			default:
				printf("0x%02x ",buf[loop]);
				break;
		}
	}
	printf("\n");
}













