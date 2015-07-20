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

/**************************************************
**brief api模式发送AT指令
**param atcmd  指向命令字符串的指针
**      pparam 指向参数的指针
**      len    参数的长度
**reval 
**************************************************/
int XBeeSendATCmd(int8* atcmd,uint8* pparam,uint8 len)
{
  uint8 wbuf[256],i;
  XBeeApiATCmdType *cmd = (XBeeApiATCmdType*)wbuf;
  cmd->start_delimiter  = 0x7E;
  cmd->len_msb          = (uint8)((4+len)>>8);
  cmd->len_lsb          = (uint8)(4+len);
  cmd->frame_type       = 0x08;
  cmd->frame_id         = 0x52;
  cmd->atCmd[0]         = *atcmd;
  cmd->atCmd[1]         = *(atcmd+1);
  for(i=0;i<len;i++)
   *(((uint8*)cmd)+7+i) = *(pparam+i);
  *(((uint8*)cmd)+7+len) = XBeeApiChecksum(((uint8*)cmd)+3,4+len); 
#if 1
  printf("api cmd is:");
  for(i=0;i<8+len;i++)
     printf("0x%02x ",*(((uint8*)cmd)+i));
  printf("\r\n");
#endif
  return WriteComPort((uint8*)cmd,8+len);
}

/*********************************************************
**biref 设置ID的值
**********************************************************/
int XBeeSetPanID(void)
{
  uint8 panID[8],i=0;
  int8 *cmd = "ID";
  for(i=0;i<8;i++)
     *(panID+i) = 0xee;
  return XBeeSendATCmd(cmd,panID,8);
}
/*********************************************************
**biref 发送读取ID值命令
**********************************************************/
int XBeeReadPanID(void)
{
  uint8 panID[1];
  int8 *cmd = "OP";
  return XBeeSendATCmd(cmd,panID,0);
}
/*********************************************************
**biref 发送AI命令
**********************************************************/
int XBeeReadAI(void)
{
  uint8 paramer[1];
  int8 *cmd = "AI";
  return XBeeSendATCmd(cmd,paramer,0);
}
/*********************************************************
**biref 发送MY命令
**********************************************************/
int XBeeSendMY(void)
{
  uint8 paramer[1];
  int8 *cmd = "MY";
  return XBeeSendATCmd(cmd,paramer,0);
}
/*************************************************************
**brief 发送SC
*************************************************************/
int XBeeSetChannel(void)
{
  uint8 paramer[8],i=1;
  int8 *cmd = "SC";
  for(i=0;i<8;i++)
     *(paramer+i) = 0x11;
  return XBeeSendATCmd(cmd,paramer,2);
}
/*************************************************************
**brief 连接/创建网络指示灯闪烁时间
**param time   it should be 0x0A-0xFF or 0x00
*************************************************************/
int XBeeSetLT(uint8 time)
{
  uint8 paramer[1];
  int8 *cmd = "LT";
  *paramer = time;
  return XBeeSendATCmd(cmd,paramer,1);
}
/*************************************************************
**brief  读取信道
*************************************************************/
int XBeeReadCH(void)
{
  uint8 paramer[0];
  int8 *cmd = "CH";
  return XBeeSendATCmd(cmd,paramer,0);
}
/*************************************************************
**brief 复位模块
*************************************************************/
int xbeeFR(void)
{
  uint8 paramer[8];
  int8 *cmd = "FR";
  *(paramer) = 0;
  return XBeeSendATCmd(cmd,paramer,0);
}
/*************************************************************
**brief 使能更改内容
*************************************************************/
int XbeeSendAC(void)
{
  uint8 paramer[8];
  int8 *cmd = "AC";
  *(paramer) = 0;
  return XBeeSendATCmd(cmd,paramer,0);
}
/*********************************************************
**biref 发送WR命令,保存更改
**********************************************************/
int XBeeSendWR(void)
{
  uint8 paramer[1];
  int8 *cmd = "WR";
  return XBeeSendATCmd(cmd,paramer,0);
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


























