#ifndef _XBEE_CONFIG_H_
#define _XBEE_CONFIG_H_


//xbee配置
#define SCAN_CHANNEL 0x4200							//xbee信道选择
#define _AR_PER 		10							//发送AR周期（s）
#define _NET_OFF_TIME  	0xff						//允许入网时间
#define _USE_CTS		1

//数据发送/接收配置
//#define _SEND_DATA_MAX				4				//最大同时发送数据包数量
//#define _SEND_DATA_TIMEOUT			300000			//数据包应答最大等待时间（us）
#define _REV_DATA_MAX           		(1024*1024)//1048576			//串口最大缓存数据（字节）
#define _SOURCE_CNT						5					//源路由接点有效期 ×2s





//#define __XBEE_TEST_LAR_NODE__	1				//大规模网络测试
#define __XBEE_TEST__ 			1				//调试用










#endif
