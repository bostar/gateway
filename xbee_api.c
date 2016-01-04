#include "xbee_api.h"
#include "xbee_routine.h"
/******************************************************************************************
**broef 创建链表
******************************************************************************************/
SourceRouterLinkType *CreatRouterLink(uint8 *mac_adr,uint16 target_adr,uint8 *mid_adr,uint8 num)
{
	SourceRouterLinkType* pRouterLink = NULL; 
	uint8 i=0;

	pRouterLink = (SourceRouterLinkType*)malloc(sizeof(SourceRouterLinkType));
	if(pRouterLink == NULL)
	{
		printf("\033[31mno enough menmary\r\n");
		return NULL;
	}
	for(i=0;i<8;i++)
		pRouterLink->mac_adr[i] = mac_adr[i];
	pRouterLink->target_adr = target_adr;
	for(i=0;i<num*2;i++)
		pRouterLink->mid_adr[i] = mid_adr[i];
	pRouterLink->num_mid_adr = num;
	pRouterLink->next = NULL;
	return pRouterLink;
}
/******************************************************************************************
**broef 创建节点
******************************************************************************************/
SourceRouterLinkType *CreatNode(uint8 *mac_adr,uint8 *target_adr)
{
	SourceRouterLinkType* pRouterLink = NULL; 
	uint8 i=0;
 	 
	pRouterLink = (SourceRouterLinkType*)malloc(sizeof(SourceRouterLinkType));
	if(pRouterLink == NULL)
	{
		printf("\033[31mno enough menmary\r\n");
		return NULL;
	}
	for(i=0;i<8;i++)
		pRouterLink->mac_adr[i] = mac_adr[i];
	pRouterLink->target_adr = 0;
	pRouterLink->target_adr |= target_adr[0]<<8;
	pRouterLink->target_adr |= target_adr[1];
	for(i=0;i<40;i++)
		pRouterLink->mid_adr[i] = 0;
	pRouterLink->num_mid_adr = 0;
	//pRouterLink->lock_state = 0;
#if __XBEE_TEST_LAR_NODE__
	pRouterLink->send_cmd_times = 0;
	pRouterLink->rev_rep_times = 0;
#endif
	pRouterLink->next = NULL;
	return pRouterLink;
}
/*******************************************************************************************
**brief 查找第n个节点
**param
**reval NULL 没有数据
      P   数据地址
*******************************************************************************************/
SourceRouterLinkType *FindnNode(SourceRouterLinkType* const pNode,uint16 n)
{
	SourceRouterLinkType *p=NULL,*pS=NULL;
	uint8 i=1;
	p = pNode;
	while(p != NULL)
	{
		if(i == n)
			return p;
		pS = p;
		p = p->next;
		i++;
	}
	return NULL;
}
/*******************************************************************************************
**brief 查找数据,网络地址
**param
**reval NULL 没有数据
		P	 数据地址
*******************************************************************************************/
SourceRouterLinkType *FindNetAdr(SourceRouterLinkType* const pNode,uint16 target_adr)
{
	SourceRouterLinkType *p;
	p = pNode;
	while(p != NULL && p->target_adr != target_adr)
	{
		p = p->next;
	}
	return p;
}
/*******************************************************************************************
**brief 查找数据，物理地址
**param
**reval NULL 没有数据
		P	 数据地址
*******************************************************************************************/
SourceRouterLinkType *FindMacAdr(SourceRouterLinkType* const pNode,uint8 *mac_adr)
{
	SourceRouterLinkType *p=NULL;
	p = pNode;
	while(p != NULL && arrncmp(p->mac_adr,mac_adr,8) != 0)
	{
		p = p->next;
	}
	return p;
}
/*******************************************************************************************
**brief 插入数据
*******************************************************************************************/
uint8 AddData(SourceRouterLinkType* const pNode,SourceRouterLinkType *pNodeS)
{
	SourceRouterLinkType *p=NULL;
	p = pNode;
	while(p->next != NULL)
	{
		p = p->next;
	}
	p->next = pNodeS;
	//LinkPrintf(pLinkHead);
	return 0;
}
/*******************************************************************************************
**brief 链表长度
param	头指针
reval	链表长度
*******************************************************************************************/
uint16 LinkLenth(SourceRouterLinkType* const pNode)
{
	SourceRouterLinkType *p;
	uint8 i;
	p = pNode;
	i = 0;
	while(p != NULL)
	{
		p = p->next;
		i++;
	}
	return i;
}
/*******************************************************************************************
**brief 删除链表节点
**param pNode 链表头
		deleteNode 删除的节点
××reval 0 删除成功   1节点不存在
*******************************************************************************************/
uint8 DeleteNode(SourceRouterLinkType* const pNode,SourceRouterLinkType *deleteNode)
{
	SourceRouterLinkType *p=NULL,*pS=NULL;
	p = pNode;
	while(p != NULL && p != deleteNode)
	{
		pS = p;
		p = p->next;
	}	
	if(p == NULL)
		return 1;	//节点不存在
	if(p == pNode)
		return 2;	//表头禁止被删除
	pS->next = p->next;
	free(p);
	p = NULL;
	deleteNode = NULL;
	//LinkPrintf(pLinkHead);
	return 0;
}
/********************************************************************************************
**brief 对比节点路径是否相同
××param
××reval 1	节点路径发生变化，需要重新写入
		0	节点不需要保存
		2	不是同一个节点
********************************************************************************************/
uint8 compareNode(SourceRouterLinkType *pNode,SourceRouterLinkType *pNodeS)
{
	SourceRouterLinkType *p=NULL,*pS=NULL;
	
	p = pNode;
	pS = pNodeS;
	if(arrncmp(p->mac_adr,pS->mac_adr,8) != 0)
		return 2;		//不是同一个节点
	if(p->target_adr != pS->target_adr)
		return 1;		//路径发生变化，更新路径
	if(p->num_mid_adr != pS->num_mid_adr)
		return 1;		//路径发生变化，更新路径
	if(p->num_mid_adr != 0)
	{
		if(arrncmp(p->mid_adr,pS->mid_adr,p->num_mid_adr*2) != 0)
			return 1;		//路径发生变化，更新路径
	}	
	return 0;			
}
/********************************************************************************************
**breief 打印单个节点信息
********************************************************************************************/
void NodePrintf(SourceRouterLinkType *pNode)
{
	uint8 i;
	for(i=0;i<8;i++)
	{
		printf("%02x ",pNode->mac_adr[i]);
	}
	printf("  ");
	printf("0x%04x	",pNode->target_adr);
	printf("  %d	",pNode->num_mid_adr);
	for(i=0;i<pNode->num_mid_adr*2;i++)
	{
		if(i%2 == 0)
			printf("0x%02x",pNode->mid_adr[i]);
		else
			printf("%02x ",pNode->mid_adr[i]);
	}
	printf("\n");
}
/********************************************************************************************
**breief 打印全部链表
********************************************************************************************/
void LinkPrintf(SourceRouterLinkType *pNode)
{
	uint8 cnt,i;
	SourceRouterLinkType *p=NULL;

	cnt = LinkLenth(pNode);
	printf("\033[35m");
	printf("编号	");
	printf("目标物理地址		");
	printf("  目标网络地址 ");
	printf("数量");
	printf(" 中间节点地址\n");
	p = pNode;
	for(i=1;i<=cnt;i++)
	{
		printf(" %d	",i);
		NodePrintf(p);
		p = p->next;
	}
	printf("\033[0m");
}
/*************************************************************
**brief	比较数组是否相等
*************************************************************/
int8 arrncmp(uint8 *arr1,uint8 *arr2,uint8 n)
{
	uint8 i;
	for(i=0;i<n;i++)
	{
		if(*(arr1+i) != *(arr2+i))
			return 1;
	}
	return 0;
}
/*lock list info***************************************************************************************************************/
/****************************************************************
**broef 创建链表
****************************************************************/
LockListInfoType *creat_lock_list_info(void)
{
	LockListInfoType *p=NULL;

	p = (LockListInfoType*)malloc(sizeof(LockListInfoType));
	if(p == NULL)
	{
		puts("error");
		return NULL;
	}
	return p;
}
/****************************************************************
**broef 创建节点
****************************************************************/
LockListInfoType *creat_lock_list_info_node(uint8 *mac_adr,uint8 *net_adr,uint8 *sen_data)
{
	LockListInfoType *p=NULL;
	uint8 i=0;
	p = (LockListInfoType*)malloc(sizeof(LockListInfoType));
	if(p ==NULL)
	{
		puts("error");
		return NULL;
	}
	for(i=0;i<8;i++)
		p->mac_adr[i] = *(mac_adr+i);
	p->net_adr[0] = *(net_adr);
	p->net_adr[1] = *(net_adr+1);
	p->statex = (((uint16)sen_data[0] << 8) | sen_data[1]);
	p->statey = (((uint16)sen_data[2] << 8) | sen_data[3]);
	p->statez = (((uint16)sen_data[4] << 8) | sen_data[5]);
	p->next = NULL;
	return p;
}
/*******************************************************************************************
**brief 查找数据，物理地址
**param mac_adr	指向物理地址的指针
**reval NULL 没有数据
		P	 数据地址
*******************************************************************************************/
LockListInfoType *find_node_mac(LockListInfoType* const Node,uint8 *mac_adr)
{
	LockListInfoType *p=NULL;

	p = Node;
	while(p != NULL && arrncmp(p->mac_adr,mac_adr,8) != 0)
	{
		p = p->next;
	}
	return p;
}
/*******************************************************************************************
**brief 插入数据
*******************************************************************************************/
uint8 add_node(LockListInfoType* const pNode,LockListInfoType *pNodeS)
{
	LockListInfoType *p=NULL;
	p = pNode;
	while(p->next != NULL)
	{
		p = p->next;
	}
	p->next = pNodeS;
	return 0;
}
/*******************************************************************************************
**brief 删除链表节点
**param pNode 链表头
		deleteNode 删除的节点
××reval 0 删除成功   1节点不存在
*******************************************************************************************/
uint8 del_lock_list_node(LockListInfoType* const pNode,LockListInfoType *deleteNode)
{
	LockListInfoType *p=NULL,*pS=NULL;
	p = pNode;
	while(p != NULL && p != deleteNode)
	{
		pS = p;
		p = p->next;
	}
	if(p == NULL)
		return 1;	//节点不存在
	if(p == pNode)
		return 2;	//表头禁止被删除
	pS->next = p->next;
	free(p);
	p = NULL;
	deleteNode = NULL;
	//LinkPrintf(pLinkHead);
	return 0;
}
/*******************************************************************************************
**brief 链表长度
param	头指针
reval	链表长度
*******************************************************************************************/
uint16 lock_list_len(LockListInfoType* const pNode)
{
	LockListInfoType *p=NULL;
	uint8 i;
	p = pNode;
	i = 0;
	while(p != NULL)
	{
		i++;
		p = p->next;
	}
	return i;
}
/*******************************************************************************************
**brief 链表数据写入文件
param	头指针
*******************************************************************************************/
int write_lock_list(LockListInfoType* const pNode)
{
	int fd=-1,new_offset=-1;
	LockListInfoType *p=NULL;
	uint8 buf[125],i;
	uint16 line_num=1,net_adr;
	uint64 mac_adr;
	
	fd = open("lock_info_list.txt", O_WRONLY | O_CREAT | O_TRUNC, 0x666);
	if(fd < 0)
	{
		printf("open %s failed\n","lock_info_list.txt");
		return -1;
	}
	if((new_offset = lseek(fd,0,SEEK_END)) < 0)
	{
		printf("seek %s failed\n","lock_info_list.txt");
		return -1;
	}
	snprintf((int8*)buf,sizeof("num  net_adr mac_adr          senerx senery senerz\r\n"),"num  net_adr mac_adr          senerx senery senerz\r\n");
	if((write(fd , buf , sizeof(buf))) < 0)
	{
		printf("write %s failed\r\n","lock_info_list.txt");
		return -1;
	}
	p = pNode;
	while(p != NULL)
	{
		if((new_offset = lseek(fd,0,SEEK_END)) < 0)
		{
			printf("seek %s failed\n","lock_info_list.txt");
			return -1;
		}
		net_adr = p->net_adr[0]<<8 | p->net_adr[1];
		mac_adr = 0;
		for(i=0;i<8;i++)
			mac_adr = mac_adr | (uint32)p->mac_adr[7-i]<<(8*i);
		snprintf((int8*)buf,sizeof("%04d %04x %llx             %04d %04d %04d\r\n"),"%04d %04x %llx             %04d %04d %04d\r\n",line_num,net_adr,mac_adr,p->statex,p->statey,p->statez);
		if((write(fd , buf , sizeof(buf))) < 0)
		{
			printf("write %s failed\r\n","lock_info_list.txt");
			return -1;
		}
		line_num++;
		p = p->next;
	}
	fsync(fd);
	close(fd);
	return 0;
}

/*serial send data buffer define***********************************************************************************************/
/****************************************************************
**brief push data into a queue
****************************************************************/
void queue_push_in(uint8 *mac_adr,uint16 net_adr,uint8 *data ,uint16 len ,uint8 req)
{
	XBeeDataWaiteSendType *p;
	uint16 i;
	p = (XBeeDataWaiteSendType*)malloc(sizeof(XBeeDataWaiteSendType));
	if(p == NULL)
	{
		printf("\033[31mno enough menmary\r\n");
		return;
	}
	for(i=0;i<8;i++)
		p->mac_adr[i] = *(mac_adr+i);
	p->net_adr[0] = (uint8)net_adr;
	p->net_adr[1] = (uint8)(net_adr>>8);
	for(i=0;i<len;i++)
		p->data[i] = *(data+i);
	p->data_len = len;
	p->req = req;
	TAILQ_INSERT_TAIL(&waite_send_head, p, tailq_entry);
}
/****************************************************************
**brief push data out of a queue
****************************************************************/
void queue_push_out(void)
{
	
}

/*serial receive data buffer define*******************************************************************************/
void creat_circular_queue( CircularQueueType *queue )  
{  
    queue -> front = 0;  
    queue -> rear = 0;  
    queue -> count = 0;  
    queue -> maxsize = _REV_DATA_MAX - 1;  
}
bool is_empty( CircularQueueType *queue)    
{
    if(queue->count == 0)  
        return true;  
    else  
        return false;  
}  
bool is_full( CircularQueueType *queue )  
{  
    if ( queue->count == _REV_DATA_MAX )
        return true;  
    else  
        return false;  
} 
bool in_queue( CircularQueueType *queue, uint8 value)  
{  
    if ( queue -> count == _REV_DATA_MAX )
    {  
		queue -> elements[queue ->rear] = value;  
		queue -> rear = (queue -> rear + 1) % _REV_DATA_MAX;
		queue -> front = queue -> rear;
		return false;
    }  
    else  
    {  
		queue -> elements[queue ->rear] = value;  
		queue -> rear = (queue -> rear + 1) % _REV_DATA_MAX;       	
       	queue -> count++;
		return true;         
	}	    
}  
bool out_queue(CircularQueueType *queue , uint8 *out_buf)  
{
    if ( queue -> count == 0 )
    {  
		return false;
    }  
    else  
    {  
		*out_buf = queue -> elements[queue -> front];
        queue -> front = (queue -> front + 1) % _REV_DATA_MAX;  
        queue -> count -= 1;
        return true;
    }  
}
void clear_queue(CircularQueueType *queue)
{
	queue->rear = queue->front = 0;
	queue->count=0;
}
/**********************************************************************
**brief	write n byte into queue 
**param	p_cqueue	pointer of the circular queue
		buf			the write data buffer
		n			the number of byte to be writen
**reval	the number of byte writen into queue
**********************************************************************/
uint16 write_cqueue(CircularQueueType* p_cqueue , uint8* buf , uint16 n)
{
	uint16 i=0,reval=0;
	bool state;

	for(i=0;i<n;i++)
	{
		state = in_queue( p_cqueue, *(buf + i));
		reval++;
	}
	return reval;
}
/**********************************************************************
**brief	read n byte from queue 
**param	p_cqueue	pointer of the circular queue
		buf			the read data buffer
		n			the number of byte to be read
**reval	the number of byte read from queue
**********************************************************************/
uint16 read_cqueue(CircularQueueType* p_cqueue , uint8* buf , uint16 n)
{
	uint8 i=0,reval=0;
	bool state;

	for(i=0;i<n;i++)
	{
		state = out_queue( p_cqueue , buf+i);
		if(state == true)
		{
			//printf("\033[32m%02x \033[0m",*(buf+i));
			reval++;
			//printf("%02x ",*(rbuf+i));
		}
		else
			break;
	}
	//if(reval > 0)
		//printf("\033[35mreval = %d \033[0m",reval);
	return reval;
}

void print_queue(CircularQueueType *queue)
{
	uint32 cnt=0,i=0;

	cnt = queue->count;
	while(cnt)
	{
		printf("%02x ",queue -> elements[(queue -> front + i) % _REV_DATA_MAX]);
		cnt--;
		i++;
	}
}
/**********************************************************************************************************************************/
/***********************************************************************
**brief 创建发送应答链表节点
***********************************************************************/
transReqListType *creat_trans_req_node(void)
{
	transReqListType *p=NULL;
	uint8 i;

	p = (transReqListType*)malloc(sizeof(transReqListType));
	if(p == NULL)
	{
		printf("\033[31no enough mmomory \033[0m\r\n");
		return NULL;
	}
	for(i=0;i<8;i++)
		p->mac_adr[i] = 0;
	p->net_adr[0] = 0;
	p->net_adr[1] = 0;
	for(i=0;i<10;i++)
		p->data[i] = 0;
	p->len = 0;
	p->state = false;
	p->next = NULL;
	return p;
}
/***********************************************************************
**brief 创建发送应答链表
***********************************************************************/
transReqListType *creat_trans_req_list(void)
{
	return creat_trans_req_node();
}








































