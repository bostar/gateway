//#include "xbee_include.h"

#include "xbee_routine.h"
#include "xbee_api.h"
#include "serial.h"
#include "sys/queue.h"
#include <time.h>
/**************************gloable variable************************/
CircularQueueType serial_rbuf;
SourceRouterLinkType *pLinkHead=NULL;
uint8 send_xbee_state=0;
uint32 waite_send_head_num=0;
//uint32 send_data_timeout=0;

/*************************** mutex ********************************/
pthread_mutex_t mutex01_serial_rbuf = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex02_pLinkHead = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex03_send_xbee_state = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex04_waite_send_head_num = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex05_send_data_timeout = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex06_waite_send_head = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex07_CoorInfo = PTHREAD_MUTEX_INITIALIZER;

/*************************** cond *********************************/
pthread_cond_t cond_send_xbee=PTHREAD_COND_INITIALIZER;

void xbee_routine_thread(void)
{
	int ret=0;
    pthread_t id;
	uint8 _i,_adr[8];
	uint8 *HeadMidAdr=NULL;

	xbee_gpio_init();
	xbee_serial_port_init();

	pthread_mutex_lock(&mutex06_waite_send_head);
	TAILQ_INIT(&waite_send_head);
	pthread_mutex_unlock(&mutex06_waite_send_head);

	for(_i=0;_i<8;_i++)
		_adr[_i] = 0;
	pthread_mutex_lock(&mutex02_pLinkHead);
	pLinkHead = CreatRouterLink(_adr,0,HeadMidAdr,0); //创建路由路径链表
	pthread_mutex_unlock(&mutex02_pLinkHead);

	pthread_mutex_lock(&mutex01_serial_rbuf);
	creat_circular_queue( &serial_rbuf );
	pthread_mutex_unlock(&mutex01_serial_rbuf);

	printf("\033[33mstart xbee_routine_thread_send_data...\033[0m\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_send_data,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread_send_data error!n");
    }
	printf("\033[33mstart xbee_routine_thread_timer...\033[0m\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_timer,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread_timer error!n");
    }
	printf("\033[33mstart xbee_routine_thread_read_serial...\033[0m\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_read_serial,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread_read_serial error!n");
    }

	XBeeNetInit();

	printf("\033[33mstart xbee_routine_thread_process_serial_buf...\033[0m\r\n");
    ret=pthread_create(&id,NULL,(void *) xbee_routine_thread_process_serial_buf,NULL);
    if(ret!=0){
        printf ("Create xbee_routine_thread_process_serial_buff error!n");
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
		pthread_mutex_lock(&mutex04_waite_send_head_num);
		printf("waite_send_head_num = %d\n",waite_send_head_num);
		pthread_mutex_unlock(&mutex04_waite_send_head_num);
		usleep(1000000);
	}
}

void xbee_routine_thread_process_serial_buf(void)
{
	static uint8 rbuf[255]={0};
	static int16 len=0;
	while(1)
	{
		len = read_one_package(rbuf);
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
				case transmit_status:
					//ProcessTranState();
					pthread_mutex_lock(&mutex03_send_xbee_state);
					send_xbee_state--;
					printf("\033[33msend_xbee_state = %d\033[0m",send_xbee_state);
					pthread_cond_signal(&cond_send_xbee);
					pthread_mutex_unlock(&mutex03_send_xbee_state);
					break;
				default:
					break;
			}
		}
		usleep(1);
	 }
}

void xbee_routine_thread_send_data(void)
{
	XBeeDataWaiteSendType *p;
	uint32 temp=0;
	struct timespec time_cur;
	while(1)
	{
		pthread_mutex_lock(&mutex03_send_xbee_state);
		clock_gettime(CLOCK_MONOTONIC,&time_cur);
		time_cur.tv_nsec += 300000000;
puts("\033[33m标记1\033[0m");
		while(send_xbee_state > _SEND_DATA_MAX)
		{
			//pthread_cond_timedwait(&cond_send_xbee,&mutex03_send_xbee_state,&time_cur);
			pthread_cond_wait(&cond_send_xbee,&mutex03_send_xbee_state);   //加timeout 用绝对时间
		}
		pthread_mutex_unlock(&mutex03_send_xbee_state);  
puts("\033[33m标记2\033[0m");
		pthread_mutex_lock(&mutex04_waite_send_head_num);
		temp = waite_send_head_num;
		pthread_mutex_unlock(&mutex04_waite_send_head_num);
puts("\033[33m标记3\033[0m");
		while(temp > _QUEUE_BUF_MAX) //丢弃过量的数据包
		{
			pthread_mutex_lock(&mutex06_waite_send_head);
			p = TAILQ_FIRST(&waite_send_head);
			TAILQ_REMOVE(&waite_send_head, p, tailq_entry);
			pthread_mutex_unlock(&mutex06_waite_send_head);
puts("\033[33m标记4\033[0m");
			free(p);

			pthread_mutex_lock(&mutex04_waite_send_head_num);
			waite_send_head_num--;
			temp = waite_send_head_num;
			pthread_mutex_unlock(&mutex04_waite_send_head_num);
puts("\033[33m标记5\033[0m");
		}
		
		pthread_mutex_lock(&mutex04_waite_send_head_num);
		temp = waite_send_head_num;
		pthread_mutex_unlock(&mutex04_waite_send_head_num); 
puts("\033[33m标记6\033[0m");
		if(temp > 0) 
		{
			pthread_mutex_lock(&mutex06_waite_send_head);
			p = TAILQ_FIRST(&waite_send_head);
			XBeeTransReq(p->mac_adr,p->net_adr,p->options,p->data,p->data_len,p->req);
			TAILQ_REMOVE(&waite_send_head, p, tailq_entry);
			pthread_mutex_unlock(&mutex06_waite_send_head);
puts("\033[33m标记7\033[0m");
			free(p);

			pthread_mutex_lock(&mutex04_waite_send_head_num);
			waite_send_head_num--;
			pthread_mutex_unlock(&mutex04_waite_send_head_num);
puts("\033[33m标记8\033[0m");
			pthread_mutex_lock(&mutex03_send_xbee_state);
			send_xbee_state++;
			pthread_mutex_unlock(&mutex03_send_xbee_state);
puts("\033[33m标记9\033[0m");
		}
		
		usleep(10);
	}
}

static uint8 serial_buf[255];
void xbee_routine_thread_read_serial(void)
{
	uint32 len=0,i=0;
	while(1)
	{
		len = ReadComPort (serial_buf, 255);
		if(len > 0)
		{
			pthread_mutex_lock(&mutex01_serial_rbuf);
			for(i=0;i<len;i++)
			{
				in_queue( &serial_rbuf, *(serial_buf + i));
				//printf("\033[31m%02x \033[0m",*(serial_buf + i));
			}
			pthread_mutex_unlock(&mutex01_serial_rbuf);
		}
		usleep(10);
	}
}

void xbee_routine_thread_timer(void)
{
	while(1)
	{
		usleep(1000000);
	}
}

void xbee_routine_thread_test_lar_node(void)
{
	uint8 i;
	uint16 temp;
	SourceRouterLinkType *p;
	while(1)
	{
		pthread_mutex_lock(&mutex04_waite_send_head_num);
		printf("the packages has not send = %d\r\n",waite_send_head_num);
		pthread_mutex_unlock(&mutex04_waite_send_head_num);

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

		pthread_mutex_lock(&mutex04_waite_send_head_num);
		printf("waite to be send data packages = %d\r\n",waite_send_head_num);
		pthread_mutex_unlock(&mutex04_waite_send_head_num);

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













