#include "xbee_log.h"

int16 err_log_sigle(int8 *buf , uint16 len)
{
	int fd=-1,new_offset=-1;

	fd = open(CoorInfo.logname , O_WRONLY | O_CREAT,0x666);
	if(fd < 0)
	{
		printf("open %s failed\n",CoorInfo.logname);
		return -1;
	}
	new_offset = lseek(fd,0,SEEK_END);
	if(new_offset < 0)
	{
		printf("seek %s failed\n",CoorInfo.logname);
		return -1;
	}
	if(write(fd , buf , len) < 0)
	{
		printf("write %s failed\n",CoorInfo.logname);
		return -1;
	}
	if(write(fd , "\r\n" , 2) < 0)
	{
		printf("write %s failed\n",CoorInfo.logname);
		return -1;
	}
	fsync(fd);
	close(fd);
	return 0;
}

int16 err_log(int8 *buf , uint16 len)
{
	int16 retval=0;
	int ret = -1;
	MUTEX_LOCK(&mutex15_errlog);
	if(ret == 0)
	{
		retval = err_log_sigle(buf , len);
	}
	MUTEX_UNLOCK(&mutex15_errlog);
	return retval;
}




























