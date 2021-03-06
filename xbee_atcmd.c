/*********************************************************
**描述    ：XBee api指令帧及控制命令
**版本    ：V1.0
**日期    ：2015.07.08
*********************************************************/
#include "xbee_atcmd.h"
#include "string.h"
#include "xbee_bsp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <limits.h>
#include "serial.h"
#include "pthread.h"
#include "xbee_routine.h"

//#define USE_PRINTF

/**************************************************
**brief api模式发送AT指令
**param atcmd  指向命令字符串的指针
**      pparam 指向参数的指针
**      len    参数的长度
**reval 
**************************************************/
int16 XBeeSendATCmd(int8* atcmd,uint8* pparam,uint8 len,uint8 IsRes)
{
	static uint8 wbuf[128];
	uint8 i;

	XBeeApiATCmdType *cmd = (XBeeApiATCmdType*)wbuf;
	cmd->start_delimiter  = 0x7E;
	cmd->len_msb          = (uint8)((4+len)>>8);
	cmd->len_lsb          = (uint8)(4+len);
	cmd->frame_type       = 0x08;
	cmd->frame_id         = IsRes;
	cmd->atCmd[0]         = *atcmd;
	cmd->atCmd[1]         = *(atcmd+1);
	for(i=0;i<len;i++)
		*(((uint8*)cmd)+7+i) = *(pparam+i);
	*(((uint8*)cmd)+7+len) = XBeeApiChecksum(((uint8*)cmd)+3,4+len);

	MUTEX_LOCK(&mutex10_serial_wbuf);
	write_cqueue( &serial_wbuf , (uint8*)cmd , 8+len );
	MUTEX_UNLOCK(&mutex10_serial_wbuf);
	return (8+len);
}

/************************************************************
**brief xbee发送数据请求
**param adr 指向64位地址的指针
        net_adr 指向16位网络地址的指针
        options 配置字
        rf_data 待发送数据的指针
        len     待发送数据的字节数
        IsRes   是否应答
**reval 发送的字节数
************************************************************/
int16 XBeeTransReq(uint8 *adr,uint8 *net_adr,SetOptions options,uint8 *rf_data,uint16 len,IsResp IsRes)
{
  	static uint8 wbuf[138];
	uint8 cnt=0;
  	XBeeTransReqType *frame = (XBeeTransReqType*)wbuf;

  	frame->start_delimiter = 0x7E;
	frame->len_msb         = (uint8)((14+len)>>8);
  	frame->len_lsb         = (uint8)(14+len);
  	frame->frame_type      = 0x10;
  	frame->frame_id        = IsRes;
  	for(cnt=0;cnt<8;cnt++)
    	frame->adr[cnt] = *(adr + cnt);
  	frame->net_adr[0]      = *(net_adr);
  	frame->net_adr[1]      = *(net_adr+1);
  	frame->readius         = 0;
  	frame->options         = options;
  	for(cnt=0;cnt<len;cnt++)
    	*((uint8*)frame + 17 + cnt) = *(rf_data + cnt);
  	*(((uint8*)frame)+17+len) = XBeeApiChecksum(((uint8*)frame)+3,14+len);

	SourceRouterLinkType *p=NULL;
	int16 n=0;
	uint8 re_buf[128];

	
	MUTEX_LOCK(&mutex13_pSourcePathList);
	p = FindMacAdr(pSourcePathList , adr);
	if(p != NULL)
	{
		n = XBeeCreatSourceRout( p->mac_adr, p->target_adr , p->num_mid_adr , p->mid_adr , re_buf);
	}
	MUTEX_UNLOCK(&mutex13_pSourcePathList);

	MUTEX_LOCK(&mutex12_trans_req_buf);
	write_cqueue(&trans_req_buf , re_buf , n);
	write_cqueue(&trans_req_buf , (uint8*)frame , 18+len);
	MUTEX_UNLOCK(&mutex12_trans_req_buf);

	return (n + len + 18);
}
/*********************************************************
**brief 创建源路由帧
**param mac_adr 目标的物理地址
		net_adr 目标网络地址 比如 0xEEFF 对应网络地址 0xEE 0xFF
		num 目标与发送者间的节点数量
		mid_adr 指向中间节点网络地址的指针 
			比如 从发送者A到目标E的节点顺序为 A B C D E
			中间节点的网络地址	B	0xAABB
							C	0xCCDD
							D	0xEEFF
			输入参数的排列顺序为	EE FF CC DD AA BB 
		re_buf	命令帧数据返回指针
*********************************************************/
int16 XBeeCreatSourceRout(uint8 *mac_adr,uint16 net_adr,uint16 num,uint8 *mid_adr,uint8* re_buf)
{
	static uint8 wbuf_temp[128];
	uint8 wbuf_len=0,i=0;
	uint16 lenth=0;
	
	wbuf_len = 18 + num*2;
	lenth = wbuf_len - 4;
	*(wbuf_temp) = 0x7E;		
	*(wbuf_temp + 1) = (uint8)(lenth >> 8);
	*(wbuf_temp + 2) = (uint8)lenth;
	*(wbuf_temp + 3) = 0x21;
	*(wbuf_temp + 4) = 0;
	for(i=0;i<8;i++)
		*(wbuf_temp + i +5) = *(mac_adr+i);
	*(wbuf_temp + 13) = (uint8)(net_adr >> 8);
	*(wbuf_temp + 14) = (uint8)net_adr;
	*(wbuf_temp + 15) = 0;
	*(wbuf_temp + 16) = num;  	
	for(i=0;i<num*2;i++)
		 *(wbuf_temp + 17 + i) = *(mid_adr + i);
	*(wbuf_temp + wbuf_len -1) = XBeeApiChecksum(wbuf_temp + 3, wbuf_len - 4);
	
	for(i=0;i<wbuf_len;i++)
		re_buf[i] = wbuf_temp[i];

	return wbuf_len;
}
/*********************************************************
**brief 远程模块参数控制
**param mac_adr 目标的物理地址
		net_adr 目标网络地址 比如 0xEEFF 对应网络地址 0xEE 0xFF
		option	命令选项
		cmd		命令的指针
		papram	命令参数指针
		len		参数长度
		IsRes   是否应答
*********************************************************/
int16 XBeeRemoteATCmd(uint8 *mac_adr , uint8 *net_adr , RemoteATRequestType option , int8 *cmd , uint8 *param , uint8 len , IsResp IsRes)
{
	static uint8 buf[128],i;
	uint16 lenth;

	*buf = 0x7e;
	lenth = len +15;
	*(buf + 1) = (uint8)(lenth >> 8);
	*(buf + 2) = (uint8)(lenth);
	*(buf + 3) = remote_AT_command_request;
	*(buf + 4) = IsRes;
	for(i=0;i<8;i++)
		*(buf + 5 + i) = *(mac_adr + i);
	for(i=0;i<2;i++)
		*(buf + 13 + i) = *(net_adr + i);
	*(buf + 15) = option;
	*(buf + 16) = (uint8)*cmd;
	*(buf + 17) = (uint8)*(cmd +1 );
	for(i=0;i<len;i++)
		*(buf + 18 + i) = *(param + i);
	*(buf + lenth + 3) = XBeeApiChecksum(buf +3 , lenth);

	SourceRouterLinkType *p=NULL;
	int16 n=0;
	uint8 re_buf[128];

	MUTEX_LOCK(&mutex13_pSourcePathList);
	p = FindMacAdr(pSourcePathList , mac_adr);
	if(p != NULL)
	{
		n = XBeeCreatSourceRout( p->mac_adr, p->target_adr , p->num_mid_adr , p->mid_adr , re_buf);
	}
	MUTEX_UNLOCK(&mutex13_pSourcePathList);
	
	MUTEX_LOCK(&mutex12_trans_req_buf);
	write_cqueue(&trans_req_buf , re_buf , n);
	write_cqueue(&trans_req_buf , (uint8*)buf , lenth+4);
	MUTEX_UNLOCK(&mutex12_trans_req_buf);
#if 0
	for(i=0;i<n;i++)
		printf("%02x ",*(re_buf+i));
	puts("");
	for(i=0;i<lenth+4;i++)
		printf("%02x ",*(buf+i));
	puts("");
#endif
	return (lenth+4);
}
/*********************************************************
**biref 设置ID的值
**********************************************************/
int16 XBeeSetPanID(uint8 *panID,uint8 IsRes)
{
  	int8 *cmd = "ID";
  	return XBeeSendATCmd(cmd,panID,8,IsRes);
}
/*********************************************************
**biref 设置ZS的值
**********************************************************/
int16 XBeeSetZS(uint8 data,uint8 IsRes)
{
  	uint8 panID[1];
  	int8 *cmd = "ZS";
  	*panID = data;
  	return XBeeSendATCmd(cmd,panID,1,IsRes);
}
/*********************************************************
**biref 发送读取ID值命令
**********************************************************/
int16 XBeeReadPanID(void)
{
  	uint8 panID[1];
  	int8 *cmd = "OP";
  	return XBeeSendATCmd(cmd,panID,0,RES);
}
/*********************************************************
**biref 发送AI命令
**********************************************************/
int16 XBeeReadAI(void)
{
  	uint8 paramer[1];
  	int8 *cmd = "AI";
  	return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref 发送MY命令
**********************************************************/
int16 XBeeSendMY(uint8 IsRes)
{
  	uint8 paramer[1];
  	int8 *cmd = "MY";
  	return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*************************************************************
**brief 发送SC
*************************************************************/
int16 XBeeSetChannel(uint16 channel,uint8 IsRes)
{
	uint8 paramer[2];
	int8 *cmd = "SC";
	*paramer     = (uint8)(channel >> 8);
	*(paramer+1) = (uint8)(channel);
	return XBeeSendATCmd(cmd,paramer,2,IsRes);
}
/*************************************************************
**brief 连接/创建网络指示灯闪烁时间
**param time   it should be 0x0A-0xFF or 0x00
*************************************************************/
int16 XBeeSetLT(uint8 time,uint8 IsRes)
{
  	uint8 paramer[1];
  	int8 *cmd = "LT";
  	*paramer = time;
  	return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*************************************************************
**brief  读取信道
*************************************************************/
int16 XBeeReadCH(uint8 IsRes)
{
  	uint8 paramer[0];
  	int8 *cmd = "CH";
  	return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*************************************************************
**brief 复位模块
*************************************************************/
int16 xbeeFR(uint8 IsRes)
{
  	uint8 paramer[8];
  	int8 *cmd = "FR";
  	*(paramer) = 0;
  	return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*************************************************************
**brief 使能更改内容
*************************************************************/
int16 XbeeSendAC(uint8 IsRes)
{
  	uint8 paramer[8];
  	int8 *cmd = "AC";
  	*(paramer) = 0;
  	return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*********************************************************
**biref 发送WR命令,保存更改
********************************************ad**/
int16 XBeeSendWR(uint8 IsRes)
{
  	uint8 paramer[1];
  	int8 *cmd = "WR";
 	return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*********************************************************
**biref 发送NC命令,查看可加入子设备数量
**********************************************************/
int16 XBeeSendNC(uint8 IsRes)
{
  	uint8 paramer[1];
  	int8 *cmd = "NC";
  	return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*********************************************************
**biref 发送SH命令
**********************************************************/
int16 XBeeReadSH()
{
  	uint8 paramer[1];
  	int8 *cmd = "SH";
  	paramer[1] = 0;
  	return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref 发送SL命令
**********************************************************/
int16 XBeeReadSL()
{
  	uint8 paramer[1];
  	int8 *cmd = "SL";
  	paramer[1] = 0;
  	return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref 发送SM命令 休眠设置
**********************************************************/
int16 XBeeSendSM(SleepType sleep,IsResp IsRes)
{
  	uint8 paramer[1];
  	int8 *cmd = "SM";
  	paramer[0]=sleep;
  	return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref 发送设置NJ命令 用以区分当前模块类型
**********************************************************/
int16 XBeeSetNJ(uint8 time,uint8 IsRes)
{
  	uint8 paramer[1];
  	int8 *cmd = "NJ";
  	paramer[0]=time;	
  	return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref 重启网络
**param data 0 重启单个网络
			 1 重启全部网络
**********************************************************/
int16 XBeeSetNR(uint8 data,IsResp IsRes)
{
	uint8 paramer[1];
  	int8 *cmd = "NR";
	paramer[0]=data;
  	return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref 发送SP命令 休眠时间
**********************************************************/
int16 XBeeSetSP(uint16 time,IsResp IsRes)
{
  uint8 paramer[2];
  int8 *cmd = "SP";
  paramer[0]=(uint8)(time>>8);
  paramer[1]=(uint8)time;
  return XBeeSendATCmd(cmd,paramer,2,IsRes);
}
/*********************************************************
**biref 发送SN命令 休眠时间个数
**********************************************************/
int16 XBeeSetSN(uint16 time,IsResp IsRes)
{
  uint8 paramer[2];
  int8 *cmd = "SN";
  paramer[0]=(uint8)(time>>8);
  paramer[1]=(uint8)time;
  return XBeeSendATCmd(cmd,paramer,2,IsRes);
}
/*********************************************************
**biref 发送ST命令 唤醒时间
**********************************************************/
int16 XBeeSetST(uint16 time,IsResp IsRes)
{
  uint8 paramer[2];
  int8 *cmd = "ST";
  paramer[0]=(uint8)(time>>8);
  paramer[1]=(uint8)time;
  return XBeeSendATCmd(cmd,paramer,2,IsRes);
}
/*********************************************************
**brief 设置AR
*********************************************************/
int16 XBeeSetAR(uint8 data,IsResp IsRes)
{
	uint8 paramer[2];
	int8 *cmd = "AR";
  	paramer[0]=data;
  	return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref 发送读取SM命令
**********************************************************/
int16 XBeeReadSM(void)
{
	uint8 paramer[1];
  	int8 *cmd = "SM";
  	paramer[0]=0;
  	return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref 发送读取NJ命令
**********************************************************/
int16 XBeeReadNJ(void)
{
	uint8 paramer[1];
  	int8 *cmd = "NJ";
  	paramer[0]=0;
  	return XBeeSendATCmd(cmd,paramer,0,RES);	
}
/*********************************************************
**biref set baud rate 115200
**********************************************************/
int16 XBeeSetBD(uint32 bd)
{
	uint8 param[1];
	int8 *cmd = "BD";
	if(bd == 1200)
		*param = 0;
	else if(bd == 2400)
		*param = 1;
	else if(bd == 4800)
		*param = 2;
	else if(bd == 9600)
		*param = 3;
	else if(bd == 19200)
		*param = 4;
	else if(bd == 38400)
		*param = 5;
	else if(bd == 57600)
		*param = 6;
	else if(bd == 115200)
		*param = 7;
	return XBeeSendATCmd(cmd,param,1,RES);
}
/*********************************************************
**biref 发送读取AT参数命令
**********************************************************/
int16 XBeeReadAT(int8 *at_cmd)
{
	uint8 paramer[1];
	int8 *cmd;
	cmd = at_cmd;
	paramer[0]=0;
	return XBeeSendATCmd(cmd,paramer,0,RES);	
}
/*********************************************************
**biref 发送设置AT参数命令
**********************************************************/
int16 XBeeSendAT(int8 *at_cmd)
{
	uint8 paramer[1];
  	int8 *cmd;
	cmd = at_cmd;
  	paramer[0]=0;
  	return XBeeSendATCmd(cmd,paramer,0,NO_RES);	
}
/*********************************************************
**biref 设置AT参数命令
**********************************************************/
uint16 XBeeSetAT(int8 *at_cmd, uint8 *param, uint8 len, IsResp IsRes)
{
	int8 *cmd;
	cmd = at_cmd;
	return XBeeSendATCmd(cmd,param,len,IsRes);
}
/********************************************************
**brief 发送广播
********************************************************/
int16 XBeeBoardcastTrans(uint8 *data,uint16 len,IsResp IsRes)
{
	uint8 cnt;
	uint8 adr[8],net_adr[2];

	for(cnt=0;cnt<6;cnt++)
    	adr[cnt] = 0;
	adr[6] = 0xFF;
	adr[7] = 0xFF;
	net_adr[0] = 0xff;
	net_adr[1] = 0xfe;

	return XBeeTransReq(adr,net_adr,Default,data,len,IsRes);
}
/********************************************************
**brief 单播发送
********************************************************/
int16 XBeeUnicastTrans(uint8 *adr,uint8 *net_adr,SetOptions options,uint8 *rf_data,uint16 len,IsResp IsRes)
{
	return XBeeTransReq(adr,net_adr,options,rf_data,len,IsRes); 
}

uint8 XBeeApiChecksum(uint8 *begin,uint16 length)
{
  	uint8 sum = 0,i = 0;
  	for(i = 0;i < length;i++)
  	{
    	sum += begin[i];
  	}
  	return (0xff - sum);
}


























