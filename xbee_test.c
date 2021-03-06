#include "xbee_test.h"
#include "xbee_vari_type.h"
#include "xbee_api.h"
#include <stdlib.h>
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
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include<sys/types.h>
#include<sys/stat.h>

#if __XBEE_TEST_LAR_NODE__
int WrLogTxt(void)
{
	static char w_buf[256];
	char title[] = "编号 长地址                  网络地址 发数据数 接收数   发送/接收差 通信成功率 \n";
	char filename[] = "/mnt/gateway/test_Log.txt";
	int fd=-1,res,cur;
	char i,j;
	SourceRouterLinkType *p;

	//remove("test_Log.txt");
	fd = open(filename , O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IRWXO);
	if(fd < 0)
	{
		printf("打开失败\n");
		return -1;
	}
	cur = ftruncate(fd, 0);
	if(cur < 0)
		return -1;
	lseek(fd, 0, SEEK_SET);
	res = write(fd,title,sizeof(title));
	cur = lseek(fd,-1,SEEK_CUR);
	if(res < 0)
		printf("open file failed, errno=%d.\n", errno);
	for(i=1;i<=LinkLenth(pLinkHead);i++)
	{
		p = FindnNode(pLinkHead,(uint8)i);
		sprintf(w_buf,"%04d ",i);
		res = write(fd,w_buf,5);

		for(j=0;j<8;j++)
			sprintf(w_buf+j*3,"%02x ",p->mac_adr[(uint8)j]);
		res = write(fd,w_buf,24);

		sprintf(w_buf,"0x%04x   ",p->target_adr);
		res = write(fd,w_buf,9);

		sprintf(w_buf,"%08d ",p->send_cmd_times);
		res = write(fd,w_buf,9);

		sprintf(w_buf,"%08d ",p->rev_rep_times);
		res = write(fd,w_buf,9);

		//sprintf(w_buf,"   %02d   ",p->num_mid_adr);
		//res = write(fd,w_buf,8);

		//if(p->send_cmd_times >= p->rev_rep_times)
			sprintf(w_buf," %08d    ",p->send_cmd_times - p->rev_rep_times);
		//else
			//sprintf(w_buf," %08d    ",0);
		res = write(fd,w_buf,13);
		
		if(p->send_cmd_times > p->rev_rep_times)
			sprintf(w_buf," %.2f%%    ",(float)(p->rev_rep_times+1)/(float)p->send_cmd_times*100);
		else
			sprintf(w_buf," %.2f%%    ",(float)100);
		res = write(fd,w_buf,11);
		
		//sprintf(w_buf," %04d    ",p->join_net_times);
		//res = write(fd,w_buf,9);

		//time_t now;    //实例化time_t结构
		//struct tm  *timenow;    //实例化tm结构指针
		//timenow = localtime(&now);
		//res = write(fd,asctime(timenow),24);
		res = write(fd,"\n",1);
		//cur = lseek(fd,-1,SEEK_CUR);
	}
	fsync(fd);
	close(fd);
	return res;
}
#endif
#if __XBEE_TEST_LAR_NODE__
int printt_log(void)
{
	char filename[] = "/mnt/gateway/test_Log.txt";
	int fd=-1;

	fd = open(filename , O_RDONLY);
	if(fd < 0)
	{
		printf("open file \"%s\" failed, errno=%d.\n",filename, errno);
        return -1;
	}
	return 0;
}
#endif
#if __XBEE_TEST_LAR_NODE__
int ts_log(void)
{
	char filename[] = "/mnt/gateway/qts_Log.txt";
	int fd=-1,res=0,cur;
	char wbuf[50],buf[20],i;
	uint16 len;

	fd = open(filename , O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IRWXO);
	if(fd < 0)
	{
		printf("打开失败\n");
		return -1;
	}
	cur = lseek(fd,0,SEEK_END);
	if(cur < 0)
	{
		printf("确定位置失败！\n");
		return -1;
	}
	do
	{
		len = read_one_package_f_queue( &ts_buf , (uint8*)buf );
		if(len)
		{
			for(i=0;i<len;i++)
				sprintf(wbuf+3*i,"%02x ",buf[(uint8)i]);
			printf("%s \n",wbuf);
			res = write(fd,wbuf,len*3);
			res = write(fd,"\n",1);
		}
	}while(len);
	fsync(fd);
	close(fd);
	return res;
}
#endif

















































































































