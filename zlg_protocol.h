#ifndef __ZLG_PROTOCOL_H__
#define __ZLG_PROTOCOL_H__

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
void communicate_thread(void);
void heart_beat_thread(void *arg);

void ackRegisterNetwork(unsigned short NetAddress,ackCmd_t cmd,unsigned short panid,unsigned char channel);
//void changeNodeType(devTypeCmd_t deviceType);
//void changePanidChannel(unsigned short panid,unsigned char channel);
//void resetAllNode(void);
void testLink(const char * ieeeAddress);
void restoreFactoryConfig(unsigned short DstAddr);
void heartbeat(const unsigned short *needRequestAddresses,unsigned char nodes);
void startSensorCalibration(void);
void testBeep(unsigned short DstAddr,unsigned char cmd);
void testLed(unsigned short DstAddr,unsigned char ioLevel);
void testMotor(unsigned short DstAddr,unsigned char cmd);
void switchLockControl(unsigned short DstAddr,unsigned char cmd);

#endif
