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
int XBeeSendATCmd(uint8 *atcmd,uint8* pparam,uint8 len)
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
  for(i=0;i<8+len;i++)
     printf("0x%02x ",*(((uint8*)cmd)+i));
  printf("\r\n");
#endif
  return WriteComPort((uint8*)cmd,8+len);
}

/*********************************************************
**biref 设置ID的值
**param state = 1  set ID
**           != 1  read ID
**reval
**********************************************************/
int XBeePanID(void)
{
  uint8 panID[8],i=0;
  uint8 cmd[2];
  uint8 *str = "ID";
  for(i=0;i<2;i++)
     sprintf(cmd+i,"%02x",str);
 /* cmd[0]=0x73;
  cmd[1]=0x68;*/
  for(i=0;i<8;i++)
     *(panID+i) = 0;
  return XBeeSendATCmd(cmd,panID,8);
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


























