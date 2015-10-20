#include <unistd.h>
#include <stdio.h>
#include "xbee_bsp.h"
#include "xbee_atcmd.h"
#include "xbee_protocol.h"
#include "xbee_routine.h"
#include <pthread.h>
#include <string.h>
#include "xbee_api.h"
#include "server_duty.h"
#include <time.h>
#include "xbee_test.h"

uint8 rbuf[255];
int16 len;
SourceRouterLinkType *pLinkHead=NULL;
uint8 *HeadMidAdr=NULL;
pthread_mutex_t xbee_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t xbee_send_data_timer_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_send_xbee=PTHREAD_COND_INITIALIZER;

uint8 send_xbee_state=0;
uint32 waite_send_head_num=0;
uint32 send_data_timeout=0;

void xbee_routine_thread(void)
{
	int ret=0;
    pthread_t id;

	uint8 _i,_adr[8];
	xbee_gpio_init();
	xbee_serial_port_init();

	pthread_mutex_lock(&xbee_mutex);
	XBeeInit();
	for(_i=0;_i<8;_i++)
		_adr[_i] = 0;
	pLinkHead = CreatRouterLink(_adr,0,HeadMidAdr,0);    //创建路由路径链表
	TAILQ_INIT(&waite_send_head);
	pthread_mutex_unlock(&xbee_mutex);

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
		pthread_mutex_lock(&xbee_mutex);
		len = UartRevDataProcess(rbuf);
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
					send_xbee_state--;
					pthread_cond_signal(&cond_send_xbee);
					break;
				default:
					break;
			}
		}
		pthread_mutex_unlock(&xbee_mutex);
		usleep(1);
	 }
}

void xbee_routine_thread_send_data(void)
{
	XBeeDataWaiteSendType *p;
	static uint32 timeout=0;
	while(1)
	{
		pthread_mutex_lock(&xbee_mutex);
		while(send_xbee_state > _SEND_DATA_MAX || send_data_timeout > _SEND_DATA_TIMEOUT)
			pthread_cond_wait(&cond_send_xbee,&xbee_mutex);
		pthread_mutex_lock(&xbee_send_data_timer_mutex);
		send_data_timeout = 0;
		pthread_mutex_unlock(&xbee_send_data_timer_mutex);
		while(waite_send_head_num > _QUEUE_BUF_MAX) //丢弃过量的数据包
		{
			p = TAILQ_FIRST(&waite_send_head);
			TAILQ_REMOVE(&waite_send_head, p, tailq_entry);
			free(p);
			waite_send_head_num--;
		}
		if(waite_send_head_num > 0) 
		{
			p = TAILQ_FIRST(&waite_send_head);
			XBeeTransReq(p->mac_adr,p->net_adr,p->options,p->data,p->data_len,p->req);
			TAILQ_REMOVE(&waite_send_head, p, tailq_entry);
			free(p);
			waite_send_head_num--;
			send_xbee_state++;	
		}
		pthread_mutex_unlock(&xbee_mutex);
		usleep(1);
	}
}

void xbee_routine_thread_timer(void)
{
	while(1)
	{
		pthread_mutex_lock(&xbee_send_data_timer_mutex);
		send_data_timeout++;
		pthread_mutex_unlock(&xbee_send_data_timer_mutex);
		usleep(1);
	}
}

void xbee_routine_thread_test_lar_node(void)
{
	uint8 i;
	uint16 temp;
	SourceRouterLinkType *p;
	while(1)
	{
		pthread_mutex_lock(&xbee_mutex);
		printf("the packages has not send = %d\r\n",waite_send_head_num);
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
				//waite_send_head_num++;
			}
		}
		printf("waite to be send data packages = %d\r\n",waite_send_head_num);
		WrLogTxt();
		pthread_mutex_unlock(&xbee_mutex);
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
			LinkPrintf(pLinkHead);
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













