#include "xbee_log.h"

int16 err_log(int8 *buf , uint16 len)
{
	int fd=-1,new_offset=-1;

	fd = open(CoorInfo.logname , O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IRWXO);
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






























