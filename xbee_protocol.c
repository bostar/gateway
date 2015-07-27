#include "xbee_protocol.h"
#include "xbee_atcmd.h"


uint8 UartRevDataProcess(void)
{
	uint8 UartRevBuf[255];
	int16 UartRevLen;
	uint16 DataLen,i;
	uint8 checksum,FrameType,temp;

	UartRevLen =  xbee_serial_port_read(UartRevBuf);
	if(UartRevLen == 0)
		goto TheEnd;
	if(UartRevBuf[0] != 0x7E)
		goto TheEnd;
	temp = UartRevBuf[1];
	i = (uint16)temp << 8;
	temp = UartRevBuf[2];
	DataLen = i + (uint16)UartRevBuf[2];
	checksum = XBeeApiChecksum(&UartRevBuf[3],DataLen); //校验数据
	if(checksum != UartRevBuf[DataLen+4])
		goto TheEnd;
	FrameType = UartRevBuf[3];
	switch(FrameType)
	{
		case 1:
			break;
		default:
			break;
	}
	TheEnd: return 0;
}
























