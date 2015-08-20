/*********************************************************
**描述    ：XBee API帧结构
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
  uint8 wbuf[256],i;
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
#if defined USE_PRINTF
  printf("api cmd is:");
  for(i=0;i<8+len;i++)
     printf("0x%02x ",*(((uint8*)cmd)+i));
  printf("\r\n");
#endif
  return WriteComPort((uint8*)cmd,8+len);
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
  	uint8 wbuf[256],cnt=0;
  	XBeeTransReqType *frame = (XBeeTransReqType*)wbuf;
  
  	frame->start_delimiter = 0x7E;
	frame->len_msb         = (uint8)((14+len)>>8);
  	frame->len_lsb         = (uint8)(14+len);
  	frame->frame_type      = 0x10;
  	frame->frame_id        = IsRes;
  	for(cnt=0;cnt<8;cnt++)
    	frame->adr[cnt] = *(adr + cnt);
#if defined USE_PRINTF
	printf("api cmd is:");
	for(cnt=0;cnt<20;cnt++)
		printf("0x%02x ",*(wbuf+cnt));
	printf("\r\n");
#endif

  	frame->net_adr[0]      = *(net_adr);
  	frame->net_adr[1]      = *(net_adr+1);
#if defined USE_PRINTF
	printf("api cmd is:");
	for(cnt=0;cnt<20;cnt++)
		printf("0x%02x ",*(wbuf+cnt));
	printf("\r\n");
#endif

  	frame->readius         = 0;
  	frame->options         = options;
  	for(cnt=0;cnt<len;cnt++)
    	*((uint8*)frame + 17 + cnt) = *(rf_data + cnt);
  	*(((uint8*)frame)+17+len) = XBeeApiChecksum(((uint8*)frame)+3,14+len);
#if 1
  	printf("发送的数据帧:");
  	for(cnt=0;cnt<18+len;cnt++)
    	printf("0x%02x ",*(((uint8*)frame)+cnt));
  	printf("\r\n");
#endif
  	return WriteComPort((uint8*)frame,18+len);
}



/*********************************************************
**biref 设置ID的值
**********************************************************/
int16 XBeeSetPanID(uint8 IsRes)
{
  	uint8 panID[8],i=0;
  	int8 *cmd = "ID";
  	for(i=0;i<8;i++)
    	*(panID+i) = 00;
  	return XBeeSendATCmd(cmd,panID,8,IsRes);
}
/*********************************************************
**biref 设置ZS的值
**********************************************************/
int16 XBeeSetZS(uint8 IsRes)
{
  	uint8 panID[1];
  	int8 *cmd = "ZS";
  	*panID = 1;
  	return XBeeSendATCmd(cmd,panID,1,IsRes);
}
/*********************************************************
**biref 发送读取ID值命令
**********************************************************/
int16 XBeeReadPanID(uint8 IsRes)
{
  	uint8 panID[1];
  	int8 *cmd = "OP";
  	return XBeeSendATCmd(cmd,panID,0,IsRes);
}
/*********************************************************
**biref 发送AI命令
**********************************************************/
int16 XBeeReadAI(uint8 IsRes)
{
  	uint8 paramer[1];
  	int8 *cmd = "AI";
  	return XBeeSendATCmd(cmd,paramer,0,IsRes);
}
/*********************************************************
**biref 发送NJ命令
**********************************************************/
int16 XBeeSetNJ(uint8 time, uint8 IsRes)
{
  	uint8 paramer[1];
  	int8 *cmd = "NJ";
  	paramer[0]=time;
  	return XBeeSendATCmd(cmd,paramer,0,IsRes);
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
int16 XBeeSetChannel(uint8 IsRes)
{
  	uint8 paramer[8],i=1;
  	int8 *cmd = "SC";
  	for(i=0;i<8;i++)
    	*(paramer+i) = 0x0B;
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
**********************************************************/
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
int16 XBeeSendSH()
{
  	uint8 paramer[1];
  	int8 *cmd = "SH";
  	paramer[1] = 0;
  	return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref 发送SL命令
**********************************************************/
int16 XBeeSendSL()
{
  	uint8 paramer[1];
  	int8 *cmd = "SL";
  	paramer[1] = 0;
  	return XBeeSendATCmd(cmd,paramer,0,RES);
}
/*********************************************************
**biref 发送SM命令 休眠设置
**********************************************************/
uint16 XBeeSendSM(SleepType sleep,IsResp IsRes)
{
  uint8 paramer[1];
  int8 *cmd = "SM";
  paramer[0]=sleep;
  return XBeeSendATCmd(cmd,paramer,1,IsRes);
}
/*********************************************************
**biref 发送SP命令 休眠时间
**********************************************************/
uint16 XBeeSendSP(uint16 time,IsResp IsRes)
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
uint16 XBeeSendSN(uint16 time,IsResp IsRes)
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
uint16 XBeeSendST(uint16 time,IsResp IsRes)
{
  uint8 paramer[2];
  int8 *cmd = "ST";
  paramer[0]=(uint8)(time>>8);
  paramer[1]=(uint8)time;
  return XBeeSendATCmd(cmd,paramer,2,IsRes);
}
/*********************************************************
**biref 发送读取SM命令 用以区分当前模块类型
**********************************************************/
uint16 XBeeReadSM(void)
{
  uint8 paramer[1];
  int8 *cmd = "SM";
  paramer[0]=0;
  return XBeeSendATCmd(cmd,paramer,0,RES);
}
/********************************************************
**brief 发送广播
********************************************************/
int16 XBeeBoardcastTrans(uint8 *data,uint16 len,IsResp IsRes)
{
	uint8 adr[8],net_adr[2],cnt;
	for(cnt=0;cnt<8;cnt++)
    	adr[cnt] = 0;
	adr[6]     = 0xFF;
	adr[7]     = 0xFF;
	net_adr[0] = 0xff;
	net_adr[1] = 0xfe;
	return XBeeTransReq(adr,net_adr,ExtTimeout,data,len,IsRes);
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
  	i = 0xff - sum;
  	return i;
}


























