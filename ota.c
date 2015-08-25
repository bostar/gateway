#include "ota.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> 
#include <stdlib.h>
#include "zlg_cmd.h"
#include "serial.h"
#include "server_duty.h"
#include <semaphore.h> //包含信号量相关头文件

sem_t ota_over;
sem_t ota_begin;

#define fireware_file_path	"/mnt/SimpleBLEPeripheral.bin"

void sendtonode(unsigned short destaddr,unsigned char *data,int len)
{
    //send_data_to_remote_node(destaddr,data,len);
    WriteComPort(data, len);
    printf("Send over \r\n");

}

static unsigned short crc16(unsigned short crc, unsigned char val)
{
  const unsigned short poly = 0x1021;
  unsigned char cnt;
  unsigned char msb = 0;
  for (cnt = 0; cnt < 8; cnt++, val <<= 1)
  {
    msb = (crc & 0x8000) ? 1 : 0;
    //printf("msb = %02x\r\n",msb);
    //usleep(1000);
    crc <<= 1;
    if (val & 0x80)  crc |= 0x0001;
    if (msb)         crc ^= poly;
  }

  return crc;
}

void ota_thread(void)
{
    int fd;int len;
    unsigned int loop,loop1;
    unsigned short pkgnum,crcword;
    //unsigned char destaddr[2];
    unsigned char wbuf[255];
    unsigned char *oat_data = malloc(256 * 1024);
    if(oat_data == NULL)
    {
        printf("[OTA]:malloc error!\r\n");
        return;
    }
    sem_init(&ota_begin,0,0); //对信号量进行初始化，第一个0表示此信号量子整个进程中共享，第二个1表示信号量初始值
    sem_init(&ota_over,0,0);

    while(1)
    {
        sem_wait(&ota_begin);
        fd = open(fireware_file_path,O_RDONLY);
        if(fd < 0)
        {
            printf("[OTA]:open file error!\r\n");
            return;
        }
        len = read(fd,oat_data ,256 * 1024); 
        printf("[OTA]:first 16 byte is :\r\n");
        for(loop = 0;loop < 16;loop ++)
        {
            printf("%02x",oat_data[loop]);
        }
        pkgnum = (len % 64)?(len / 64 + 1): (len / 64);
        printf("\r\n");
        printf("[OTA]:file length is %d bytes; sum of pkg is %d\r\n",len,pkgnum);
        printf("[OTA]:set byte which was not used as 0xff\r\n");
        for(loop = len;loop < ((unsigned int)pkgnum * 64);loop ++)
        {
            printf("[OTA]:file one byte\r\n");
            oat_data[loop] = 0xff;
        }

        /* calc crc */
        crcword = 0;
        for(loop = 0;loop < ((unsigned int)pkgnum * 64);loop ++)
        {
            if((loop >= 0x90) && (loop < 0x90 + 4))
            {
            }
            else
            {
                crcword = crc16(crcword,oat_data[loop]);
            }
        }
    
        printf("[OTA]:crcword is %d;loop = %d\r\n",crcword,loop);
        crcword = crc16(crcword,0);
        printf("[OTA]:crcword is %d\r\n",crcword);
        crcword = crc16(crcword,0);
        printf("[OTA]:crcword is %d\r\n",crcword);
#if 1
        while(!networking_over())
        {
            printf("[OTA]:wait untile all node is online\r\n");
            usleep(5 * 1000000);
        }
        /* ota begin */
        printf("[OTA]:ota begin ...\r\n");
        wbuf[0] = 'O';
        wbuf[1] = 'T';
        wbuf[2] = 'A';
        wbuf[3] = 0x00; // cmd word
        *(unsigned short *)&wbuf[4] = *(unsigned short *)&oat_data[94]; // version
        printf("[OTA]:fireware version is 0x%04x\r\n",*(unsigned short *)&wbuf[4]);
        wbuf[6] = (unsigned char)pkgnum; // pkgnum low byte
        wbuf[7] = (unsigned char)(pkgnum >> 8); // pkgnum high byte
        
        for(loop = 0;loop < 5;loop ++)
        {
            WriteComPort(wbuf, 8);
            usleep(1000000);
        }
#endif
#if 1    
        /* transfer ota datas ... */
        for(loop = 0;loop < pkgnum;loop ++)
        {
            wbuf[0] = 0xfe;
            wbuf[1] = 0x42;
            wbuf[2] = 0x4d;
            wbuf[3] = 0x01; // cmd word
            wbuf[4] = (unsigned char)(loop * 0x10); // addr low byte
            wbuf[5] = (unsigned char)((loop * 0x10) >> 8); // addr high byte
            memcpy(&wbuf[6],&oat_data[loop * 64],64);
            wbuf[70] = 0;
            for(loop1 = 1;loop1 < 70;loop1 ++)
            {
                wbuf[70] ^= wbuf[loop1]; // end byte
            }
            //sendtonode(0xffff,wbuf,71);
            WriteComPort(wbuf, 71);
            printf("[OTA]:pkg %d over\r\n",loop);
            usleep(100000);
        }
#endif
        /* image enable and reset */
        wbuf[0] = 0xfe;
        wbuf[1] = 0x02;
        wbuf[2] = 0x4d;
        wbuf[3] = 0x03; // cmd word
        //wbuf[4] = (unsigned char)pkgnum;
        //wbuf[5] = (unsigned char)(pkgnum >> 8);
        wbuf[4] = (unsigned char)crcword;
        wbuf[5] = (unsigned char)(crcword >> 8);
        wbuf[6] = 0x00;
        for(loop = 1;loop < 6;loop ++)
        {
            wbuf[6] ^= wbuf[loop]; // end byte
        }
        sendtonode(0xffff,wbuf,7);
        sem_post(&ota_over);
    }
}
