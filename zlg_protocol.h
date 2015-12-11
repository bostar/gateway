#ifndef __ZLG_PROTOCOL_H__
#define __ZLG_PROTOCOL_H__

#define NONE         "\033[m" 
#define RED          "\033[0;32;31m" 
#define LIGHT_RED    "\033[1;31m" 
#define GREEN        "\033[0;32;32m" 
#define LIGHT_GREEN  "\033[1;32m" 
#define BLUE         "\033[0;32;34m" 
#define LIGHT_BLUE   "\033[1;34m" 
#define DARY_GRAY    "\033[1;30m" 
#define CYAN         "\033[0;36m" 
#define LIGHT_CYAN   "\033[1;36m" 
#define PURPLE       "\033[0;35m" 
#define LIGHT_PURPLE "\033[1;35m" 
#define BROWN        "\033[0;33m" 
#define YELLOW       "\033[1;33m" 
#define LIGHT_GRAY   "\033[0;37m" 
#define WHITE        "\033[1;37m"

typedef enum ackCmd {
	Allow = 0x01,
	Refuse = 0x00
}ackCmd_t;

typedef enum devTypeCmd {
	endDevice = 0x00,
	routerDevice = 0x01
}devTypeCmd_t;

enum cfg_cmd {
	cmdCheckIn = 0x00,
	cmdAckCheckIn = 0x01,
//	cmdChangeNodeType = 0x02,
//	cmdAckChangeNodeType = 0x03,
//	cmdChangePanidChannel = 0x04,
//	cmdAllNodeReset = 0x05,
	cmdLinkTest = 0x06,
	cmdAckLinkTest = 0x07,
	cmdRestoreFactoryConfig = 0x08,
	cmdHeartBeatPkg = 0x09,
	cmdDataRequest = 0x0A
};

enum sen_cmd {
	cmdSensorCalibration = 0x00,
    	cmdEventReport = 0x01,
    	cmdBatteryRemainReport = 0x02
};

enum tst_cmd {
	cmdBeepTest = 0x00,
	cmdLedTest = 0x01,
	cmdMotorTest = 0x02
};

enum beep_cmd {
	cmdSilence = 0x00,
	cmdBuzz = 0x01
};

enum motor_cmd {
	cmdStop = 0x00,
	cmdForward = 0x01,
	cmdReverse = 0x02
};

extern unsigned char iEEEAddress[8];
extern volatile unsigned short requestAddress;
void communicate_thread(void);

void ackRegisterNetwork(unsigned short NetAddress,ackCmd_t cmd,unsigned short panid,unsigned char channel);
//void changeNodeType(devTypeCmd_t deviceType);
//void changePanidChannel(unsigned short panid,unsigned char channel);
//void resetAllNode(void);
void testLink(const char * ieeeAddress);
void restoreFactoryConfig(unsigned short DstAddr);
void heartbeat(const unsigned short *needRequestAddresses,unsigned char nodes);
void startSensorCalibration(void);
void ackEventReport(unsigned short DstAddr);
void testBeep(unsigned short DstAddr,unsigned char cmd);
void testLed(unsigned short DstAddr,unsigned char ioLevel);
void testMotor(unsigned short DstAddr,unsigned char cmd);
void switchLockControl(unsigned short DstAddr,unsigned char cmd);
void mac2str(char *str,const char *ieeeAddress);
void keep_wake_nodes(void);
int restart_all_parking_node(void);
int syn_all_parking_node(void);
void reset_node_sensor(unsigned short DstAddr);
void mark_node_last_online_time(unsigned short node_address);

#endif
