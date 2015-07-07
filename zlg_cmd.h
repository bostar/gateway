#ifndef __ZLG_CMD_H__
#define __ZLG_CMD_H__

typedef enum {
    show_enable = 0x01,
    show_disable = 0x00
}showSrcAddr_cmd_t;

typedef enum {
    unicast = 0x00,
    broadcast = 0x01
}cast_mode_t;

enum nvparamoptcmd{
    enReadLoacalCfg = 0xd1,
    enSetChannelNv = 0xd2,
    enSearchNode = 0xd4,
    enGetRemoteInfo = 0xd5,
    enModifyCfg = 0xd6,
    enResetNode = 0xd9,
    enResetCfg = 0xda
};


enum temporaryparamoptcmd{
    enSetChannel = 0xd1,
    enSetDestAddr = 0xd2,
    enShowSrcAddr = 0xd3,
    enSetIoDirection = 0xd4,
    enReadIoStatus = 0xd5,
    enSetIoStatus = 0xd6,
    enReadAdcValue = 0xd7,
    enEnterSleepMode = 0xd8,
    enSetUnicastOrBroadcast = 0xd9,
    enReadNodeRssi = 0xda
};

typedef struct _dev_info_t {
    unsigned char devName[16];
    unsigned char devPwd[16];
    unsigned char devMode;
    unsigned char devChannel;
    unsigned char devPanid[2];
    unsigned char devLoacalNetAddr[2];
    unsigned char devLoacalIEEEAddr[8];
    unsigned char devDestNetAddr[2];
    unsigned char devDestIEEEAddr[8];
    unsigned char devReserve1;
    unsigned char devPowerLevel;
    unsigned char devRetryNum;
    unsigned char devTranTimeout;       // *10ms
    unsigned char devSerialRate;
    unsigned char devSerialDataB;
    unsigned char devSerialStopB;
    unsigned char devSerialParityB;
    unsigned char devReserve2;
}dev_info_t;

typedef struct _search_info_t { 
    unsigned char deviceType[2];
    unsigned char channel;
    unsigned char rate;
    unsigned char netNum[2];
    unsigned char localAddr[2];
    unsigned char runStatus;
}search_info_t;
   
extern dev_info_t stDevInfo;
extern dev_info_t remoteDevInfo;
extern search_info_t searchInfo;

void init_zlg_zm516x(void);
void read_local_cfg(void);
//void send_data_to_remote_node(unsigned char *destAddr,unsigned char *data,int len);
////////long term setting//////////////
void set_channel_nv(unsigned char nv);
void search_node(void);
void get_remote_info(unsigned short DstAddr);
void write_remote_cfg(unsigned short DstAddr, dev_info_t *DevInfo );
void reset_node(unsigned short DstAddr);
void restore_factory_settings(unsigned short DstAddr);
//temporary setting
void set_temporary_channel(unsigned char channel);
void set_temporary_DestAddr(unsigned short DestAddr);
void set_temporary_ShowSrcAddr(showSrcAddr_cmd_t cmd);
void set_temporary_io_dir(unsigned short DstAddr,unsigned char IO_Dir);
unsigned char read_temporary_io_level(unsigned short DstAddr);
void set_temporary_io_level(unsigned short DstAddr,unsigned char IO_Level);
unsigned short read_temporary_adc_value(unsigned short DstAddr);
void enter_sleep_mode(void);
void set_temporary_cast_mode(cast_mode_t mode);
unsigned char read_temporary_node_rssi(unsigned short DstAddr);
void send_data_to_remote_node(unsigned short destAddr,unsigned char *data,int len);
#endif
