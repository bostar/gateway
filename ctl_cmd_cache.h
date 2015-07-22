#ifndef __CTL_CMD_CACHE__
#define __CTL_CMD_CACHE__

int initCtlCmdCache(void);
int putCtlCmd(unsigned short addr,unsigned char cmd);
int getCtlAddres(unsigned short *addres,unsigned char *num);
int getCtlCmd(unsigned short address,unsigned char *cmd);

#endif

