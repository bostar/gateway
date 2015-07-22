#ifndef __DUL_LINK_LIST__
#define __DUL_LINK_LIST__

#define TRUE	     0
#define FALSE       -1
#define OK           0
#define OVERFLOW    -1
#define ERROR       -1

typedef int 	Status;

typedef struct _cmd{
	unsigned short addr;
	unsigned char  cmd;
}ctl_cmd_t;

typedef ctl_cmd_t *ElemType;

typedef struct DuLNode
{
	ElemType data;
	struct DuLNode *prior,*next;	
}DuLNode,*DuLinkList;

Status InitList(DuLinkList *L);
Status DestroyList(DuLinkList *L);
Status ClearList(DuLinkList L); /* 不改变L */
int ListLength(DuLinkList L);
Status GetElem(DuLinkList L,int i,ElemType *e);
int LocateElem(DuLinkList L,ElemType e,Status(*compare)(ElemType,ElemType));
Status PriorElem(DuLinkList L,ElemType cur_e,ElemType *pre_e);
Status NextElem(DuLinkList L,ElemType cur_e,ElemType *next_e);
Status ListInsert(DuLinkList L,int i,ElemType e);
Status ListDelete(DuLinkList L,int i,ElemType *e);
void ListTraverse(DuLinkList L,void(*visit)(ElemType));
void ListTraverseBack(DuLinkList L,void(*visit)(ElemType));
#endif

