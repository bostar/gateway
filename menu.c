#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "menu.h"
#include "zlg_cmd.h"
#include "serial.h"
#include "zlg_protocol.h"
#include "ctl_cmd_cache.h"

const char menu[] = "\r\n\
+********************** ZLG CMD HELP ***************************+\r\n\
+--- Description ----+--CMD --+--- Demo ----+-- Remark--------- +\r\n\
| read local cfg     | AT+1   | AT+1        |                   |\r\n\
| search node        | AT+2   | AT+2        |                   |\r\n\
| set channel nv     | AT+3   | AT+3 25     |                   |\r\n\
| get remote info    | AT+4   | AT+4 0x2002 |                   |\r\n\
| reset node         | AT+5   | AT+5 0x2002 |                   |\r\n\
| restore factory settings    | AT+6 0x2002 |                   |\r\n\
| set temp channel   | AT+7   | AT+7 25     |                   |\r\n\
| set temp DstAddr   | AT+8   | AT+8 0x2002 |                   |\r\n\
| set temp io dir    | AT+9   | AT+9 0x2002 0x55|               |\r\n\
| read temp io level | AT+A   | AT+A 0x2002 |                   |\r\n\
| set temp io level  | AT+B   | AT+B 0x2002 0x55|               |\r\n\
| read temp adc value| AT+C   | AT+C 0x2002 |                   |\r\n\
| set temp cast mode | AT+D   | AT+D 0x00   | broadcast:0x01    |\r\n\
| send to remote node| AT+S   | AT+S 0x2002 hello |             |\r\n\
| send               | send   | send hello  |                   |\r\n\
| help               | ?      |             |                   |\r\n\
+--------------------+--------+-------------+-------------------+\r\n\
| link test          | AT+T   | AT+T        |                   |\r\n\
| restore factory cfg| AT+R   | AT+R 0x0001 | 0xFFFF:restore all|\r\n\
| heart beat test    | AT+H   | AT+H        |                   |\r\n\
| end heart beat test| AT+E   | AT+E        |                   |\r\n\
| test beep          | beep   | beep 0x2001 1 |  0:silence      |\r\n\
| test leds          | leds   | leds 0x2001 0xff|               |\r\n\
| test motor         | moto   | moto 0x2001 0x00| 0x01:F 0x02:B |\r\n\
| switchlock contrl  | lock   | lock 0x2001 0x00|               |\r\n\
| test cache contrl  | cach   | cach 0x2001 0x01| 0x00:reverse  |\r\n\
+--------------------+--------+-------------+-------------------+\r\n";
const unsigned short addresses[20];
void heart_beat_thread(void *arg)
{
	unsigned char address_total;
	//putCtlCmd(0x0002,0x00);
	(void)arg;
	
	while(1)
	{
		if(!getCtlAddres(addresses,&address_total))
			heartbeat(addresses,address_total);
		usleep(600000);
//		putCtlCmd(0x0002,0x00);
		usleep(400000);
		pthread_testcancel();
	}
}

void menu_thread(void)
{
	char wbuf[255];
    	pthread_t id;
	
	while(1)
	{
		if(fgets(wbuf,255,stdin))
		{
			pthread_mutex_lock(&mut);
			wbuf[strlen(wbuf)-1] = '\0';

			if(!strncmp(wbuf,"AT+1",4))
			{
				read_local_cfg();
			}
			else if(!strncmp(wbuf,"AT+2",4))
			{
				search_node();
			}
			else if(!strncmp(wbuf,"AT+3",4))
			{
				if(!strncmp(&wbuf[strlen(wbuf)-3]," ",1))
					set_channel_nv(atoi(&wbuf[strlen(wbuf)-3]));
				else
					printf("paramter error...\r\n");							
			}
			else if(!strncmp(wbuf,"AT+4",4))
			{
				unsigned int temp;
				if(!strncmp(&wbuf[strlen(wbuf)-6],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-4],"%04x",&temp);
					get_remote_info(temp);					
				}
				else
					printf("paramter error...\r\n");							
			}
			else if(!strncmp(wbuf,"AT+5",4))
			{
				unsigned int temp;
				if(!strncmp(&wbuf[strlen((const char *)wbuf)-6],"0x",2))
				{
					sscanf(&wbuf[strlen((const char *)wbuf)-4],"%04x",&temp);
					reset_node(temp);					
				}
				else
					printf("paramter error...\r\n");
				
			}
			else if(!strncmp(wbuf,"AT+6",4))
			{
				unsigned int temp;
				if(!strncmp(&wbuf[strlen(wbuf)-6],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-4],"%04x",&temp);
					restore_factory_settings(temp);					
				}
				else
					printf("paramter error...\r\n");
			}
			else if(!strncmp(wbuf,"AT+7",4))
			{
				set_temporary_channel(atoi(&wbuf[strlen(wbuf)-3]));
			}
			else if(!strncmp(wbuf,"AT+8",4))
			{
				unsigned int temp;
				if(!strncmp(&wbuf[strlen(wbuf)-6],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-4],"%04x",&temp);
					set_temporary_DestAddr(temp);					
				}
				else
					printf("paramter error...\r\n");
			}
			else if(!strncmp(wbuf,"AT+9",4))
			{
				unsigned int temp1;
				unsigned int temp2;
				if(!strncmp(&wbuf[strlen(wbuf)-11],"0x",2) && !strncmp(&wbuf[strlen(wbuf)-4],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-9],"%04x",&temp1);
					sscanf(&wbuf[strlen(wbuf)-2],"%02x",&temp2);
					set_temporary_io_dir(temp1,temp2);					
				}
				else
					printf("paramter error...\r\n");
			}
			else if(!strncmp(wbuf,"AT+A",4))
			{
				unsigned int temp;
				if(!strncmp(&wbuf[strlen(wbuf)-6],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-4],"%04x",&temp);
					read_temporary_io_level(temp);					
				}
				else
					printf("paramter error...\r\n");				
			}
			else if(!strncmp(wbuf,"AT+B",4))
			{
				unsigned int temp1;
				unsigned int temp2;
				if(!strncmp(&wbuf[strlen(wbuf)-11],"0x",2) && !strncmp(&wbuf[strlen(wbuf)-4],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-9],"%04x",&temp1);
					sscanf(&wbuf[strlen(wbuf)-2],"%02x",&temp2);
					set_temporary_io_level(temp1,temp2);					
				}
				else
					printf("paramter error...\r\n");				
			}
			else if(!strncmp(wbuf,"AT+C",4))
			{
				unsigned int temp;
				if(!strncmp(&wbuf[strlen(wbuf)-6],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-4],"%04x",&temp);
					read_temporary_adc_value(temp);					
				}
				else
					printf("paramter error...\r\n");				
			}
			else if(!strncmp(wbuf,"AT+D",4))
			{
				unsigned int temp;
				if(!strncmp((const char *)&wbuf[strlen(wbuf)-4],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-2],"%02x",&temp);
					set_temporary_cast_mode((unsigned char)temp);					
				}
				else
					printf("paramter error...\r\n");				
			}
			else if(!strncmp(wbuf,"AT+S",4))
			{
				unsigned int temp;
				if(!strncmp(&wbuf[5],"0x",2) && (strlen(wbuf) > 12))
				{
					sscanf(&wbuf[7],"%04x",&temp);
					send_data_to_remote_node(temp,(unsigned char *)&wbuf[12],strlen(wbuf)-12);					
				}
				else
					printf("paramter error...\r\n");
			}
			
			else if(!strncmp(wbuf,"help",4) || !strncmp(wbuf,"?",1))
				printf(menu);
			else if(wbuf[0] == '\0')
			{
				//printf("zlg_zm516x > ");
				//continue;
			}
		        else if(!strncmp((const char *)wbuf,"send",4))
			{
				WriteComPort((unsigned char *)&wbuf[5],strlen(wbuf)-5);
			}
			else if(!strncmp(wbuf,"AT+T",4))
			{
				testLink((const char *)iEEEAddress);
			}
			else if(!strncmp(wbuf,"AT+R",4))
			{
				unsigned int temp;
				if(!strncmp(&wbuf[strlen(wbuf)-6],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-4],"%04x",&temp);
					restoreFactoryConfig(temp);					
				}
				else
					printf("paramter error...\r\n");							
			}
			else if(!strncmp(wbuf,"AT+H",4))
			{
				unsigned int temp;
   				int ret;
    					ret=pthread_create(&id,NULL,(void *) heart_beat_thread,&temp);
    					if(ret!=0){
        					printf ("Create heart_beat_thread error...\r\n!n");
   					}
			}
			else if(!strncmp(wbuf,"AT+E",4))
			{
				pthread_cancel(id);
			}
			else if(!strncmp(wbuf,"beep",4))
			{
				unsigned int temp1;
				unsigned char temp2;
				if(!strncmp(&wbuf[strlen(wbuf)-8],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-6],"%04x",&temp1);
					temp2 = atoi(&wbuf[strlen(wbuf)-1]);
					testBeep(temp1,temp2);					
				}
				else
					printf("paramter error...\r\n");
			}
			else if(!strncmp(wbuf,"leds",4))
			{
				unsigned int temp1;
				unsigned int temp2;
				if(!strncmp(&wbuf[strlen(wbuf)-11],"0x",2) && !strncmp(&wbuf[strlen(wbuf)-4],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-9],"%04x",&temp1);
					sscanf(&wbuf[strlen(wbuf)-2],"%02x",&temp2);
					testLed(temp1,temp2);					
				}
				else
					printf("paramter error...\r\n");
			}
			else if(!strncmp(wbuf,"moto",4))
			{
				unsigned int temp1;
				unsigned int temp2;
				if(!strncmp(&wbuf[strlen(wbuf)-11],"0x",2) && !strncmp(&wbuf[strlen(wbuf)-4],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-9],"%04x",&temp1);
					sscanf(&wbuf[strlen(wbuf)-2],"%02x",&temp2);
					testMotor(temp1,temp2);					
				}
				else
					printf("paramter error...\r\n");
			}
			else if(!strncmp(wbuf,"lock",4))
			{
				unsigned int temp1;
				unsigned int temp2;
				if(!strncmp(&wbuf[strlen(wbuf)-11],"0x",2) && !strncmp(&wbuf[strlen(wbuf)-4],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-9],"%04x",&temp1);
					sscanf(&wbuf[strlen(wbuf)-2],"%02x",&temp2);
					switchLockControl(temp1,temp2);					
				}
				else
					printf("paramter error...\r\n");
			}
			else if(!strncmp(wbuf,"cach",4))
			{
				unsigned int temp1;
				unsigned int temp2;
				if(!strncmp(&wbuf[strlen(wbuf)-11],"0x",2) && !strncmp(&wbuf[strlen(wbuf)-4],"0x",2))
				{
					sscanf(&wbuf[strlen(wbuf)-9],"%04x",&temp1);
					sscanf(&wbuf[strlen(wbuf)-2],"%02x",&temp2);
					putCtlCmd(temp1,temp2);
				}
				else
					printf("paramter error...\r\n");
			}
			else
				printf("Command not found! Input \"?\" to check commands\r\n");
			memset(wbuf,0x0,strlen(wbuf) + 1);//last is '\n'
			printf("zlg_zm516x > ");
			pthread_mutex_unlock(&mut);
		}
	}
}

