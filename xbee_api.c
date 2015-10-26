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
	for(i=0;i<8;i++)
		pRouterLink->mac_adr[i] = mac_adr[i];
	pRouterLink->target_adr = target_adr;
	for(i=0;i<num*2;i++)
		pRouterLink->mid_adr[i] = mid_adr[i];
	pRouterLink->num_mid_adr = num;
	pRouterLink->dev_type = 0;
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
	for(i=0;i<8;i++)
		pRouterLink->mac_adr[i] = mac_adr[i];
	pRouterLink->target_adr = 0;
	pRouterLink->target_adr |= target_adr[0]<<8;
	pRouterLink->target_adr |= target_adr[1];
	for(i=0;i<40;i++)
		pRouterLink->mid_adr[i] = 0;
	pRouterLink->num_mid_adr = 0;
	pRouterLink->dev_type = 0;
	pRouterLink->lock_state = 0;
	pRouterLink->send_cmd_times = 0;
	pRouterLink->rev_rep_times = 0;
	pRouterLink->next = NULL;
	return pRouterLink;
}
/*******************************************************************************************
**brief 查找第n个节点
**param
**reval NULL 没有数据
      P   数据地址
*******************************************************************************************/
SourceRouterLinkType *FindnNode(const SourceRouterLinkType *pNode,uint8 n)
{
	SourceRouterLinkType *p=NULL,*pS=NULL;
	uint8 i=1;
	p = (SourceRouterLinkType*)pNode;
	while(p != NULL && i <= n)
	{
		pS = p;
		p = p->next;
		i++;
	}
	return pS;
}
/*******************************************************************************************
**brief 查找数据,网络地址
**param
**reval NULL 没有数据
		P	 数据地址
*******************************************************************************************/
SourceRouterLinkType *FindNetAdr(const SourceRouterLinkType *pNode,uint16 target_adr)
{
	SourceRouterLinkType *p;
	p = (SourceRouterLinkType*)pNode;
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
SourceRouterLinkType *FindMacAdr(const SourceRouterLinkType *pNode,uint8 *mac_adr)
{
	SourceRouterLinkType *p=NULL;
	p = (SourceRouterLinkType*)pNode;
	while(p != NULL && arrncmp(p->mac_adr,mac_adr,8) != 0)
	{
		p = p->next;
	}
	return p;
}
/*******************************************************************************************
**brief 插入数据
*******************************************************************************************/
uint8 AddData(const SourceRouterLinkType *pNode,SourceRouterLinkType *pNodeS)
{
	SourceRouterLinkType *p=NULL;
	p = (SourceRouterLinkType*)pNode;
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
uint16 LinkLenth(const SourceRouterLinkType *pNode)
{
	SourceRouterLinkType *p;
	uint8 i;
	p = (SourceRouterLinkType*)pNode;
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
uint8 DeleteNode(const SourceRouterLinkType *pNode,SourceRouterLinkType *deleteNode)
{
	SourceRouterLinkType *p=NULL,*pS=NULL;
	p = (SourceRouterLinkType*)pNode;
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
	printf("0x");
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

//serial send data buffer define******************************************************************
/****************************************************************
**brief push data into a queue
****************************************************************/
void queue_push_in(uint8 *mac_adr,uint16 net_adr,uint8 *data ,uint16 len ,uint8 req)
{
	XBeeDataWaiteSendType *p;
	uint16 i;
	p = (XBeeDataWaiteSendType*)malloc(sizeof(XBeeDataWaiteSendType));
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

//serial receive data buffer define******************************************************************
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
			reval++;
			//printf("%02x ",*(rbuf+i));
		}
		else
			break;
	}
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












































