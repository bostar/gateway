#ifndef __SENSOR_DATA_FLOW_H__
#define __SENSOR_DATA_FLOW_H__
#include <pthread.h>
#define LOCK_NUM  5 

#define FILE_DIR "/root/data.dat"
#define sensor_data_base 0x0000

typedef short int16;
typedef char int8;
typedef unsigned short uint16;
typedef unsigned char uint8;

pthread_mutex_t sensor_mutex;
pthread_cond_t sensor_cond;

typedef struct _sensor_data_t {
	int16 x;
	int16 y;
	int16 z;
	uint8 isChecked;
	pthread_mutex_t mux;
} sensor_data_t;
typedef struct _sensor_data_save_t {
	int16 x;
	int16 y;
	int16 z;
	int16 threshold;
} sensor_data_save_t;

void sensor_data_flow_thread(void);
int judge_car_exist(uint16 DstAddr, int16 x, int16 y, int16 z);
int write_sensor_update(uint16 DstAddr, int16 x, int16 y, int16 z);
int write_sensor_calibration(uint16 DstAddr);
int writefile_sensor_calibration(uint16 DstAddr);

#endif

