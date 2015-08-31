#include "xbee_api.h"
#include "stdarg.h"
#include "xbee_routine.h"


/******************************************************************************************
**broef 创建链表
******************************************************************************************/
SourceRouterLinkType *CreatRouterLink(uint16 target_adr,uint8 *mid_adr,uint8 num)
{
	SourceRouterLinkType* pRouterLink = NULL; 
	uint8 i=0;

	pRouterLink = (SourceRouterLinkType*)malloc(sizeof(SourceRouterLinkType));
	pRouterLink->target_adr = target_adr;
	for(i=0;i<num*2;i++)
		pRouterLink->mid_adr[i] = mid_adr[i];
	pRouterLink->num_mid_adr = num;
	pRouterLink->next = NULL;
	return pRouterLink;
}
/*******************************************************************************************
**brief 查找数据
**param
**reval NULL 没有数据
		P	 数据地址
*******************************************************************************************/
SourceRouterLinkType *FindData(const SourceRouterLinkType *pNode,uint16 target_adr)
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
**brief 插入数据
*******************************************************************************************/
uint8 AddData(const SourceRouterLinkType *pNode,SourceRouterLinkType *pNodeS)
{
	SourceRouterLinkType *p,*pS=NULL;
	uint8 i=0;
	p = (SourceRouterLinkType*)pNode;
	while(p != NULL)
	{
		pS = p;
		p = p->next;
		i++;
	}
	pS->next = pNodeS;
	LinkPrintf(pLinkHead);
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
	SourceRouterLinkType *p,*pS;
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
	pS = p->next;
	free(p);
	LinkPrintf(pLinkHead);
	return 0;
}
/********************************************************************************************
**brief 对比节点是否需要替换
××param
××reval 1	节点路径发生变化，需要重新写入
		0	节点不需要保存
********************************************************************************************/
uint8 compareNode(SourceRouterLinkType *pNode,SourceRouterLinkType *pNodeS)
{
	SourceRouterLinkType *p,*pS;
	uint8 i;
	p = pNode;
	pS = pNodeS;
	if(p->target_adr != pS->target_adr)
		return 0;		//节点不需要替换
	if(p->num_mid_adr != pS->num_mid_adr)
		return 1;		//节点需要替换
	for(i=0;i<p->num_mid_adr*2;i++)
	{
		if(p->mid_adr[i] != pS->mid_adr[i])
			return 1;	//需要替换
	}
	return 0;			//节点不需要替换
}
/********************************************************************************************
**breief 打印单个节点信息
********************************************************************************************/
void NodePrintf(SourceRouterLinkType *pNode)
{
	uint8 i;
	printf("0x%04x		",pNode->target_adr);
	printf("%d		",pNode->num_mid_adr);
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
	printf("目标网络地址	");
	printf("中间节点数量	");
	printf("中间节点地址	\n");
	p = pNode;
	for(i=1;i<=cnt;i++)
	{
		printf(" %d	",i);
		NodePrintf(p);
		p = p->next;
	}
	printf("\033[0m");
}






















