#ifndef __XBEE_AT_CMD_H__
#define __XBEE_AT_CMD_H__
#include "xbee_vari_type.h"
#include "stdarg.h"

typedef enum
{
    at_command_response         =   0x88,
    modem_status                =   0x8a,
    transmit_status             =   0x8b,
    receive_packet              =   0x90,
    route_record_indicator      =   0xa1,
    mto_route_request_indcator  =   0xa3,
    remoto_command_response     =   0x97,
    node_identification         =   0x95,
    explicit_rx_indeicator      =   0x91
}APIFrameType;

typedef enum
{
  RES       =       0x52,
  NO_RES    =       0x00
}IsResp;        //应答模式

typedef enum
{
  Disable     =     0,
  PinSleep    =     1,
  CycSleep    =     4,
  PinCyc      =     5,
}SleepType;

typedef enum
{
  Default       =  0,
  DisableACK    =  0x01,
  EnableAPS     =  0x20,
  ExtTimeout    =  0x40,
}SetOptions;

typedef struct
{
  uint8 start_delimiter;
  uint8 len_msb;
  uint8 len_lsb;
  uint8 frame_type;
  uint8 frame_id;
  uint8 atCmd[2];
}XBeeApiATCmdType;        

typedef struct
{
  uint8 start_delimiter;
  uint8 len_msb;
  uint8 len_lsb;
  uint8 frame_type;
  uint8 frame_id;
  uint8 adr[8];
  uint8 net_adr[2];
  uint8 readius;
  uint8 options;
}XBeeTransReqType;  //zb发送请求帧

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


int16 XBeeSetIO(XBeeIOParam ioparam,IOStatus state);	//配置IO口
uint8 XBeeApiChecksum(uint8 *begin,uint16 length);  //求checksum
int16 XBeeSendATCmd(int8* atcmd,uint8* pparam,uint8 len,uint8 IsRes);   //发送at指令
int16 XBeeSetPanID(uint8 IsRes);   //设置ID的值
int16 XBeeSetChannel(uint16 channel,uint8 IsRes);
int16 XbeeFR(uint8 IsRes);  //
int16 XBeeSendWR(uint8 IsRes);
int16 XBeeSendMY(uint8 IsRes);
int16 XBeeReadCH(uint8 IsRes);
int16 XbeeSendAC(uint8 IsRes);  
int16 XBeeSendNC(uint8 IsRes); 
int16 XBeeSetZS(uint8 data,uint8 IsRes);   
int16 XBeeSetNJ(uint8 time,uint8 IsRes);  //实际时间=time * 1s               
int16 XBeeSetLT(uint8 time,uint8 IsRes);
int16 XBeeSetSP(uint16 num,IsResp IsRes);
int16 XBeeSetSN(uint16 num,IsResp IsRes);
int16 XBeeSetST(uint16 num,IsResp IsRes);
int16 XBeeSetAR(uint8 data,IsResp IsRes);

int16 XBeeReadNJ(void);
int16 XBeeReadSH(void);
int16 XBeeReadSL(void); 
int16 XBeeReadAI(void);
int16 XBeeReadPanID(void);  //读取ID
int16 XBeeReadAT(int8 *at_cmd);  //发送读取xbee参数指令

int16 XBeeTransReq(uint8 *adr,uint8 *net_adr,SetOptions options,uint8 *rf_data,uint16 len,IsResp IsRes); //xbee发送数据请求
int16 XBeeBoardcastTrans(uint8 *data,uint16 len,IsResp IsRes);  
int16 XBeeUnicastTrans(uint8 *adr,uint8 *net_adr,SetOptions options,uint8 *rf_data,uint16 len,IsResp IsRes);
int16 XBeeCreatSourceRout(uint8 *mac_adr,uint16 net_adr,uint16 num,uint8 *mid_adr);













#endif
