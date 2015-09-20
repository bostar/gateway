#include "xbee_api.h"
#include "stdarg.h"
#include "xbee_routine.h"
#include <string.h>

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




















