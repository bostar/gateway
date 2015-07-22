#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include "dul_link_list.h"
#include "ctl_cmd_cache.h"
#define N 10

pthread_mutex_t list_mux;
DuLinkList q;

int compareElemAddr(ctl_cmd_t *a,ctl_cmd_t *b);

void printElem(ctl_cmd_t *e)
{
	int num;
	num = LocateElem(q,e,compareElemAddr);
	printf("No.%d:e->addr is:0x%04x,e->cmd is:0x%02x\r\n",num,e->addr,e->cmd);
}

int initCtlCmdCache(void)
{
	int error;
	error = InitList(&q);
	if(error)
		return error;
	pthread_mutex_init(&list_mux,NULL);
	return 0;
}

int compareElemAddr(ctl_cmd_t *a,ctl_cmd_t *b)
{
	if(a->addr == b->addr)
		return 1;
	else
		return 0;
}

int getCtlAddres(unsigned short *addres,unsigned char *num)
{
	int i,len,num_tmp = 0;
	ctl_cmd_t *tmp;
	tmp = (ctl_cmd_t *)malloc(sizeof(ctl_cmd_t));
	pthread_mutex_lock(&list_mux);
	len = ListLength(q);
	for(i = 1; i <= len; i++)
	{
		if(!GetElem(q,i,&tmp))
		{
			*addres = tmp->addr;
			addres ++;
			num_tmp ++;	
		}
		else
		{
			pthread_mutex_unlock(&list_mux);
			printf("get list elem error!\r\n");
			return -1;
		}
	}	
	*num = num_tmp;
	pthread_mutex_unlock(&list_mux);
	return 0;
}

int getCtlCmd(unsigned short address,unsigned char *cmd)
{
	int num;
	ctl_cmd_t *tmp;
	 	
	tmp = (ctl_cmd_t *)malloc(sizeof(ctl_cmd_t));
	tmp->addr = address;
	tmp->cmd = 0x03;

	pthread_mutex_lock(&list_mux);
	num = LocateElem(q,tmp,compareElemAddr);
	if(num)
	{
		//GetElem(q,num,&tmp);
		ListDelete(q,num,&tmp);
	       	*cmd = tmp->cmd;
	}
	else
		printf("addres not exit!\r\n");
	pthread_mutex_unlock(&list_mux);
	return 0;
}

int putCtlCmd(unsigned short addr,unsigned char cmd)
{
	int num;
	ctl_cmd_t *tmp,*dedata;
	tmp = (ctl_cmd_t *)malloc(sizeof(ctl_cmd_t));
	if(!tmp)
		return -1;
	tmp->addr = addr;
	tmp->cmd = cmd;
	
	pthread_mutex_lock(&list_mux);
	num = LocateElem(q,tmp,compareElemAddr);
	if(num)
	{
		ListDelete(q,num,&dedata);
		printf("dedata->addr=0x%04x,dedata->cmd=0x%02x\r\n",dedata->addr,dedata->cmd);
	}
	if(!ListInsert(q,1,tmp))
	{	
		pthread_mutex_unlock(&list_mux);
		return 0;
	}
	else
	{
		pthread_mutex_unlock(&list_mux);
		printf("put ctl cmd error!\r\n");
		return -1;
	}
}

