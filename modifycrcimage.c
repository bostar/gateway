//#include "ota.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
//#include <semaphore.h> //包含信号量相关头文件

#define fireware_file_path      "./SimpleBLEPeripheral.hex"

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

int c2i(char ch)  
{  
        // 如果是数字，则用数字的ASCII码减去48, 如果ch = '2' ,则 '2' - 48 = 2  
        if(isdigit(ch))  
                return ch - 48;  
  
        // 如果是字母，但不是A~F,a~f则返回  
        if( ch < 'A' || (ch > 'F' && ch < 'a') || ch > 'z' )  
                return -1;  
  
        // 如果是大写字母，则用数字的ASCII码减去55, 如果ch = 'A' ,则 'A' - 55 = 10  
        // 如果是小写字母，则用数字的ASCII码减去87, 如果ch = 'a' ,则 'a' - 87 = 10  
        if(isalpha(ch))  
                return isupper(ch) ? ch - 55 : ch - 87;  
  
        return -1;  
}  

void main(void)
{
    int fd;int len;
    FILE * destfd;
    unsigned int loop,loop1;
    unsigned short pkgnum,crcword,crcwordold = 0,version = 0,versionold = 0;
    //unsigned char destaddr[2];
    unsigned char wbuf[255];
    unsigned char *oat_data = malloc(1024 * 1024);
    if(oat_data == NULL)
    {
        printf("[OTA]:malloc error!\r\n");
        return;
    }

    while(1)
    {
        fd = open(fireware_file_path,O_RDONLY);
        if(fd < 0)
        {
            printf("[OTA]:open file error!\r\n");
            continue;
        }
        len = read(fd,oat_data,1024 * 1024);
        close(fd);
        printf("len = %d\r\n",len);
        for(loop = 0;loop < len - 9;loop ++)
        {
            if(memcmp(":10089000",&oat_data[loop],9) == 0)
            {
                printf("crc imag is 0x");
                for(loop1 = 0;loop1 < 8;loop1 ++)
                {
                    putchar(oat_data[loop + 9 + loop1]);
                }
                printf("\r\n");
                oat_data[loop + 9 + 4] = oat_data[loop + 9 + 0];
                oat_data[loop + 9 + 5] = oat_data[loop + 9 + 1];
                oat_data[loop + 9 + 6] = oat_data[loop + 9 + 2];
                oat_data[loop + 9 + 7] = oat_data[loop + 9 + 3];
                unsigned char checksum = 0;
                for(loop1 = 0;loop1 < 8 + 16 * 2;loop1 += 2)
                {
                    checksum += (c2i(oat_data[loop + 1 + loop1]) * 16 + c2i(oat_data[loop + 1 + loop1 + 1]));
                    printf("0x%02x\r\n",checksum);
                }
                checksum = 0x100 - checksum;
                unsigned char out[2] = {0,};
                const unsigned char itoa[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
                out[0] = itoa[checksum / 16];
                out[1] = itoa[checksum % 16];;
                *(unsigned short*)&oat_data[loop + 1 + 8 + 16 * 2] = out[0];
                *(unsigned short*)&oat_data[loop + 1 + 8 + 16 * 2 + 1] = out[1];
                printf("checksum = 0x");
                putchar(*(unsigned short*)&oat_data[loop + 1 + 8 + 16 * 2]);
                putchar(*(unsigned short*)&oat_data[loop + 1 + 8 + 16 * 2 + 1]);
                printf("\r\n");

                destfd = fopen("./SimpleBLEPeripheral.hex","w");
                fwrite(oat_data,len,1,destfd);
                printf("write succes\r\n");
                fclose(destfd);
                printf("save file is ok\r\n");
                break;
            }
        }
        return;
    }
}
