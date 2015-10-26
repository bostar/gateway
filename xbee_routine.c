//#include "xbee_include.h"

#include "xbee_routine.h"
#include "xbee_api.h"
#include "serial.h"
#include "sys/queue.h"
#include <time.h>
#include "serial.h"
/**************************gloable variable************************/
CircularQueueType serial_rbuf;			//the serial read buffer
SourceRouterLinkType *pLinkHead=NULL;
uint32 send_xbee_state=0;
uint32 waite_send_head_num=0;
//uint32 send_data_timeout=0;
CircularQueueType trans_status_buf;		//xbee transmit request API buffer
CircularQueueType xbee_other_api_buf;			//
CircularQueueType serial_wbuf;			//the serial write buffer
CircularQueueType trans_req_buf;		//xbee transmit status API buffer
CircularQueueType route_record_buf;		//

/*************************** mutex ********************************/
pthread_mutex_t mutex01_serial_rbuf = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex02_pLinkHead = PTHREAD_MUTEX_INITIALIZER;
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

/*************************** cond *********************************/
pthread_cond_t cond_send_xbee=PTHREAD_COND_INITIALIZER;

void xbee_routine_thread(void)
{
	int ret=0;
    pthread_t id;
	uint8 _i,_adr[8];
	uint8 *HeadMidAdr=NULL;

	xbee_gpio_init();
	//xbee_serial_port_init();

	for(_i=0;_i<8;_i++)
		_adr[_i] = 0;
	pthread_mutex_lock(&mutex02_pLinkHead);
	pLinkHead = CreatRouterLink(_adr,0,HeadMidAdr,0); //创建路由路径链表
	pthread_mutex_unlock(&mutex02_pLinkHead);

	pthread_mutex_lock(&mutex01_serial_rbuf);
	creat_circular_queue( &serial_rbuf );
	pthread_mutex_unlock(&mutex01_serial_rbuf);

	pthread_mutex_lock(&mutex08_trans_status_buf);
	creat_circular_queue( &trans_status_buf );
	pthread_mutex_unlock(&mutex08_trans_status_buf);

	pthread_mutex_lock(&mutex09_xbee_other_api_buf);
	creat_circular_queue( &xbee_other_api_buf );
	pthread_mutex_unlock(&mutex09_xbee_other_api_buf);

	pthread_mutex_lock(&mutex10_serial_wbuf);
	creat_circular_queue( &serial_wbuf );
	pthread_mutex_unlock(&mutex10_serial_wbuf);

	pthread_mutex_lock(&mutex12_trans_req_buf);
	creat_circular_queue( &trans_req_buf );
	pthread_mutex_unlock(&mutex12_trans_req_buf);

	pthread_mutex_lock(&mutex11_route_record_buf);
	creat_circular_queue( &route_record_buf );
	pthread_mutex_unlock(&mutex11_route_record_buf);

	printf("\033[33mstart xbee_routine_thread_write_serial...\033[0m\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_write_serial,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread_write_serial error!n");
    }
	printf("\033[33mstart xbee_routine_thread_read_serial...\033[0m\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_read_serial,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread_read_serial error!n");
    }
	printf("\033[33mstart xbee_routine_thread_process_serial_rbuf...\033[0m\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_process_serial_rbuf,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread_process_serial_rbuf error!n");
    }

	XBeeNetInit();	//

		printf("\033[33mstart xbee_routine_thread_process_trans_status_buf...\033[0m\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_process_trans_status_buf,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread_process_trans_status_buf error!n");
    }

	printf("\033[33mstart xbee_routine_thread_process_trans_req_buf...\033[0m\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_process_trans_req_buf,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread_process_trans_req_buf error!n");
    }

	printf("\033[33mstart xbee_routine_thread_process_route_record_buf...\033[0m\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_process_route_record_buf,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread_process_route_record_buf error!n");
    }

	printf("\033[33mstart xbee_routine_thread_process_other_api_buf...\033[0m\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_process_other_api_buf,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread_process_other_api_buf error!n");
    }
	
#if __XBEE_TEST__
	printf("\033[33mstart xbee_routine_thread_test...\033[0m\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_test,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread_test error!n");
    }
#endif
#if __XBEE_TEST_LAR_NODE__
	printf("\033[33mstart xbee_routine_thread_test_lar_node...\033[0m\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_test_lar_node,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread_test_lar_node error!n");
    }
#endif
	while(1)
	{
		pthread_mutex_lock(&mutex12_trans_req_buf);
		printf("\033[36mtrans_req_buf->count = %d \033[0m",trans_req_buf.count);
		pthread_mutex_unlock(&mutex12_trans_req_buf);

		pthread_mutex_lock(&mutex03_send_xbee_state);
		printf("\033[36msend_xbee_state = %d \033[0m\n",send_xbee_state);
		pthread_mutex_unlock(&mutex03_send_xbee_state);
		usleep(2000000);
	}
}

void xbee_routine_thread_process_other_api_buf(void)
{
	static uint8 rbuf[255]={0};
	static int16 len=0;
	while(1)
	{
		len = read_one_package_f_xbee_other_api_buff(rbuf);
		if(len)
	 	{
			//TestPrintf("1",len,rbuf);
			switch(rbuf[3])
			{
				case receive_packet:
					if(rbuf[15]=='C' && rbuf[16]=='F' && rbuf[17]=='G')
						XBeeProcessCFG(rbuf);    
					else if(rbuf[15]=='C' && rbuf[16]=='T' && rbuf[17]=='L')
						XBeeProcessCTL(rbuf);
					else if(rbuf[15]=='S' && rbuf[16]=='E' && rbuf[17]=='N')
						XBeeProcessSEN(rbuf);
					else if(rbuf[15]=='O' && rbuf[16]=='T' && rbuf[17]=='A')
					{}
					break;
				case at_command_response:
					ProcessATRes(rbuf);
					break;
				default:
					break;
			}
		}
		usleep(10);
	 }
}

void xbee_routine_thread_process_trans_status_buf(void)
{
	static uint8 rbuf[255]={0};
	static int16 len=0;
	uint8 i=0;
	while(1)
	{
		len = read_one_package_f_trans_status_buf(rbuf);
		if(len)
		{
			switch(rbuf[3])
			{
				case transmit_status:
					//ProcessTranState();
					if(*(rbuf+8) != 0 )
					{
						for(i=0;i<11;i++)
							printf("%02x ",*(rbuf+i));
						puts(" ");
					}					
					pthread_mutex_lock(&mutex03_send_xbee_state);
					if(send_xbee_state > 0)
						send_xbee_state--;
					pthread_cond_signal(&cond_send_xbee);
					pthread_mutex_unlock(&mutex03_send_xbee_state);
					break;
				default:
					break;
			}
		}
		usleep(10);
	}
}

void xbee_routine_thread_write_serial(void)
{
	static uint8 rbuf[255]={0};
	static int16 len=0;
	while(1)
	{
		len = read_one_package_f_serial_wbuf(rbuf);
		if(len)
		{
			WriteComPort(rbuf, len);
		}
		usleep(10);
	}
}

void xbee_routine_thread_read_serial(void)
{
	uint32 len=0;
	static uint8 serial_buf[255];
	while(1)
	{
		len = ReadComPort (serial_buf, 255);
		if(len > 0)
		{
			write_serial_rbuf(serial_buf,len);
		}
		usleep(10);
	}
}

void xbee_routine_thread_process_trans_req_buf(void)
{
	uint32 len=0;
	static uint8 rbuf[255];
	while(1)
	{
		pthread_mutex_lock(&mutex03_send_xbee_state);
		//struct timespec time_cur;
		//clock_gettime(CLOCK_MONOTONIC,&time_cur);
		//time_cur.tv_nsec += 300000000;
		while(send_xbee_state > _SEND_DATA_MAX)
		{
			//pthread_cond_timedwait(&cond_send_xbee,&mutex03_send_xbee_state,&time_cur);
			pthread_cond_wait(&cond_send_xbee,&mutex03_send_xbee_state);   //加timeout 用绝对时间
		}
		pthread_mutex_unlock(&mutex03_send_xbee_state);

		len = read_one_package_f_trans_req_buf(rbuf);
		if(len)
		{
			write_serial_wbuf(rbuf,len);
			pthread_mutex_lock(&mutex03_send_xbee_state);
			send_xbee_state++;
			pthread_mutex_unlock(&mutex03_send_xbee_state);
		}
		usleep(10);
	}
}

void xbee_routine_thread_process_route_record_buf(void)
{
	while(1)
	{
		
	}
}

void xbee_routine_thread_process_serial_rbuf(void)
{
	uint16 len=0;
	uint8 buf[255];

	len = read_one_package_f_serial_rbuf(buf);
	if(len > 0)
	{
		if(*(buf+3) == transmit_status)
		{
			write_trans_status_buf(buf,len);
		}
		else
		{
			write_xbee_other_api_buf(buf,len);
		}
	}
	usleep(10);
}	

void xbee_routine_thread_test_lar_node(void)
{
	uint8 i;
	uint16 temp;
	SourceRouterLinkType *p;
	while(1)
	{
		pthread_mutex_lock(&mutex02_pLinkHead);
		for(i=2;i<=LinkLenth(pLinkHead);i++)
		{
			p = FindnNode(pLinkHead,i);
			if(p != NULL)
			{
				temp = 0;
				temp |= (p->target_adr >> 8);
				temp |= (p->target_adr << 8);
				p->send_cmd_times++;
				XBeePutCtlCmd(p->mac_adr,temp,1);
			}
		}
		pthread_mutex_unlock(&mutex02_pLinkHead);

		pthread_mutex_lock(&mutex02_pLinkHead);
		WrLogTxt();
		pthread_mutex_unlock(&mutex02_pLinkHead);
		usleep(10000000);
	}
}

void xbee_routine_thread_test(void)
{
	int8 in_cmd[100];
	int reval;
	uint8 i;

	while(1)
	{
		reval = scanf("%s",in_cmd);
		if(strncmp("list",in_cmd,strlen("list")) == 0)
		{
			pthread_mutex_lock(&mutex02_pLinkHead);
			LinkPrintf(pLinkHead);
			pthread_mutex_unlock(&mutex02_pLinkHead);
		}
		else if(strncmp("op",in_cmd,strlen("op")) == 0)
		{
			XBeeReadAT("OP");
			sleep(1);
			printf("\033[35m64位panID:\033[0m");
			for(i=0;i<8;i++)
				printf("0x%02x ",CoorInfo.panID64[i]);
			printf("\n");
		}
		else if(strncmp("id",in_cmd,strlen("id")) == 0)
		{
			XBeeReadAT("ID");
			sleep(1);
			printf("\033[35m64位panID:\033[0m");
			for(i=0;i<8;i++)
				printf("0x%02x ",CoorInfo.panID64[i]);
			printf("\n");
		}
		else if(strncmp("oi",in_cmd,strlen("oi")) == 0)
		{
			XBeeReadAT("OI");
			sleep(1);
			printf("\033[35m16位panID: \033[0m0x%04x\n",CoorInfo.panID16);
		}
		else if(strncmp("nj",in_cmd,strlen("nj")) == 0)
		{
			XBeeReadAT("NJ");
			sleep(1);
			printf("\033[35m允许入网时间 \033[0m0x%02x\n",CoorInfo.nj);
		}
		else if(strncmp("ch",in_cmd,strlen("ch")) == 0)
		{
			XBeeReadAT("CH");
			sleep(1);
			printf("\033[35m网络信道: \033[0m0x%04x\n",CoorInfo.channel);
		}
		else if(strncmp("check",in_cmd,strlen("check")) == 0)
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
void TestPrintf(int8* sss,int16 lens,uint8 *buf)
{
	int loop=0;
	if((int8)*(buf+15) == 'S' && (int8)*(buf+16) == 'E' && (int8)*(buf+17) == 'N')
		return;
	printf("\033[34m数据序列--%s:数据长度--%d; 数据内容:\033[0m\n",sss,lens);
	for( loop = 0;loop < lens;loop ++)
	{
		if(*(buf+3) == 0x88)
		{
			if(loop==5 || loop==6)
				printf("\033[32m%c \033[0m",(int8)buf[loop]);
			else
				printf("0x%02x ",buf[loop]);
		}
		else if(*(buf+3) == 0x90)
		{
			if(loop==15 || loop==16 || loop==17)
				printf("\033[32m%c \033[0m",(int8)buf[loop]);
			else
				printf("0x%02x ",buf[loop]);
		}
		else if(*(buf+3) == 0xa1 && loop == 3)
		{
			printf("\033[32m0x%02x \033[0m",buf[loop]);
		}
		else if(*(buf+3) == 0x8b && loop == 3)
		{
			printf("\033[35m0x%02x \033[0m",buf[loop]);
		}
		else
			printf("0x%02x ",buf[loop]);
	}
	printf("\n");
}













