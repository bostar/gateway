#ifndef __XBEE_AT_CMD_H__
#define __XBEE_AT_CMD_H__
#include "xbee_vari_type.h"

typedef struct
{
  uint8 start_delimiter;
  uint8 len_msb;
  uint8 len_lsb;
  uint8 frame_type;
  uint8 frame_id;
  uint8 atCmd[2];
}XBeeApiATCmdType;        

typedef enum
{
  AT_ID   =    1,
  AT_NJ   =    2,
  AT_P1   =    3,
  AT_OE   =    4    //读取PANID
}ATCommandType;     //AT命令集

typedef struct       
{
  uint8 start_delimiter;
  uint8 len_msb;
  uint8 len_lsb;
  uint8 frame_type;
  uint8 frame_id;
  uint8 atCmd[2];
  uint8 param;
  uint8 checksum;
}XBeeApiIOCmd; //IO口API命令帧
typedef struct       
{
  uint8 start_delimiter;
  uint8 len_msb;
  uint8 len_lsb;
  uint8 frame_type;
  uint8 frame_id;
  uint8 atCmd[2];
  uint8 param[8];
  uint8 checksum;
}XBeeApiIDCmdType; // ID API命令帧


typedef enum    
{
  IO_P0    =       1,
  IO_P1    =       2,
  IO_P2    =       3,
  IO_P3    =       4,
  IO_D0    =       5,
  IO_D1    =       6,
  IO_D2    =       7,
  IO_D3    =       8,
  IO_D4    =       9,
  IO_D5    =      10,
  IO_D6    =      11,
  IO_D7    =      12,
  IO_D8    =      13 
}XBeeIOParam;

typedef enum
{
  High  =  1,
  Low   =  2
}IOStatus;


int XBeeSetIO(XBeeIOParam ioparam,IOStatus state);	//配置IO口
int XBeeScanParam(ATCommandType AtCmd);  //发送读取xbee参数指令
uint8 XBeeApiChecksum(uint8 *begin,uint16 length);  //求checksum
int XBeePanID(void);  //设置ID的值
int XBeeSendATCmd(uint8 *atcmd,uint8* PParam,uint8 len);  //api模式发送AT指令

#endif
