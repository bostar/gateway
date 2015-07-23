#ifndef __CTL_CMD_CACHE__
#define __CTL_CMD_CACHE__

int initCtlCmdCache(void);
int putCtlCmd(const unsigned short addr,const unsigned char cmd);
int getCtlAddres(const unsigned short *addres,unsigned char *num);
int getCtlCmd(const unsigned short address,unsigned char *cmd);

#endif

