#include "xbee_api.h"
#include "stdarg.h"

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
*******************************************************************************************/
SourceRouterLinkType *FindData(const SourceRouterLinkType *pNode,uint16 target_adr)
{
	if(pNode == NULL)
		return NULL;
	if(pNode->target_adr == target_adr)
		return (SourceRouterLinkType*)pNode;
	return FindData(pNode->next,target_adr);
}
/*******************************************************************************************
**brief 插入数据
*******************************************************************************************/
uint8 AddData(void)
{
	return 0;
}
/*******************************************************************************************
**brief 统计节点数量
*******************************************************************************************/
uint16 CountNode(const SourceRouterLinkType *pNode)
{
	if(pNode->next == NULL)
		return 0;
	return 1+ CountNode(pNode->next);
}
/*******************************************************************************************
**brief 删除链表节点
*******************************************************************************************/
void DeleteNode(SourceRouterLinkType **pNode)
{
	SourceRouterLinkType** pNext;
	
 	if(NULL == pNode || NULL == *pNode)  
        return ;  
          
    pNext = &(*pNode)->next;  
    free(*pNode);  
    DeleteNode(pNext);
}












