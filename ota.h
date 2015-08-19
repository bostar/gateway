#ifndef	_OTA_H__
#define	_OTA_H__

#include <semaphore.h> //包含信号量相关头文件

extern sem_t ota_over;
extern sem_t ota_begin;

void ota_thread(void);

#endif
