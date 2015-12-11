#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sensor_data_flow.h"
#include "zlg_protocol.h"
#include "pthread.h"
#include "server_duty.h"
#include "parking_state_management.h"
#include "file_sensor_data.h"


static sensor_data_t sensor_calibration[LOCK_NUM] = {
	{0,0,0,0,PTHREAD_MUTEX_INITIALIZER},
	{0,0,0,0,PTHREAD_MUTEX_INITIALIZER},
	{0,0,0,0,PTHREAD_MUTEX_INITIALIZER},
	{0,0,0,0,PTHREAD_MUTEX_INITIALIZER},
};

static sensor_data_t sensor_update[LOCK_NUM] = {
	{0,0,0,0,PTHREAD_MUTEX_INITIALIZER},
	{0,0,0,0,PTHREAD_MUTEX_INITIALIZER},
	{0,0,0,0,PTHREAD_MUTEX_INITIALIZER},
	{0,0,0,0,PTHREAD_MUTEX_INITIALIZER},
};

void sensor_data_flow_thread(void)
{
	int sensor_xx, sensor_yy, sensor_zz;
 	int i;	
	int16 threshold;
	sensor_data_save_t *sensor_data_save;
	pthread_mutex_init(&sensor_mutex, NULL);
	pthread_cond_init(&sensor_cond, NULL);
	for ( i = 0; i < LOCK_NUM ; i++)
	{
		sensor_data_save = readSensorData(i + 1);
		if( sensor_data_save )
		{
			sensor_calibration[i].x = sensor_data_save->x;
			sensor_calibration[i].y = sensor_data_save->y;
			sensor_calibration[i].z = sensor_data_save->z;
			sensor_calibration[i].threshold = sensor_data_save->threshold;
		}
		else
			printf(RED"read node 0x%04x sensor saved data failed\r\n"NONE, i + 1);
	}

	while(1)
	{
		pthread_cond_wait(&sensor_cond, &sensor_mutex);
		i = requestAddress -1 ;
		{
			pthread_mutex_lock(&sensor_update[i].mux);
			pthread_mutex_lock(&sensor_calibration[i].mux);
			sensor_xx = abs(sensor_update[i].x - sensor_calibration[i].x);
			sensor_yy = abs(sensor_update[i].y - sensor_calibration[i].y);
			sensor_zz = abs(sensor_update[i].z - sensor_calibration[i].z);
			pthread_mutex_unlock(&sensor_update[i].mux);
			pthread_mutex_unlock(&sensor_calibration[i].mux);

			printf(CYAN"NODE 0x%04x: sensor_xx: %d, sensor_yy: %d, sensor_zz: %d\r\n"NONE,i+1,sensor_xx,sensor_yy,sensor_zz);
			threshold = sensor_calibration[i].threshold;
			if( sensor_xx > threshold || sensor_yy > threshold || sensor_zz > threshold )
			{
				event_report((uint16)(i+1), en_vehicle_comming );
			}
			else
			{
				event_report((uint16)(i+1), en_vehicle_leave );
			}
		}
	}
}

int write_sensor_update(uint16 DstAddr, int16 x, int16 y, int16 z)
{
	int i = DstAddr - 1;
	pthread_mutex_lock(&sensor_update[i].mux);
	sensor_update[i].x = x;
	sensor_update[i].y = y;
	sensor_update[i].z = z;
	pthread_mutex_unlock(&sensor_update[i].mux);
	return 0;
}

int write_sensor_calibration(uint16 DstAddr)
{
	int i = DstAddr - 1;
	pthread_mutex_lock(&sensor_update[i].mux);
	pthread_mutex_lock(&sensor_calibration[i].mux);
	sensor_calibration[i].x = sensor_update[i].x;
	sensor_calibration[i].y = sensor_update[i].y;
	sensor_calibration[i].z = sensor_update[i].z;
	pthread_mutex_unlock(&sensor_update[i].mux);
	pthread_mutex_unlock(&sensor_calibration[i].mux);
	return 0;
}

int writefile_sensor_calibration(uint16 DstAddr)
{
	int i = DstAddr - 1;
	
	pthread_mutex_lock(&sensor_calibration[i].mux);

	sensor_data_save.x = sensor_calibration[i].x;
	sensor_data_save.y = sensor_calibration[i].y;
	sensor_data_save.z = sensor_calibration[i].z;	
	sensor_data_save.threshold = sensor_calibration[i].threshold;

	pthread_mutex_unlock(&sensor_calibration[i].mux);
	return 0;	
}

int write_file_data(void)
{
	FILE *fp;
	int i;
	if( (fp = fopen( FILE_NAME, "w" )) == NULL )
	{
		printf("open file failed\r\n");
		return -1;
	}
	fputs("\n", fp );
	fclose(fp);

	for( i = 0 ;i < LOCK_NUM; i++)
	{ 
		if(!writefile_sensor_calibration(i + 1))
		{
			if(!writeSensorData(i+1, &sensor_data_save))
				printf(BLUE"write node 0x%04x sensor data to file success\r\n"NONE,i+1);
			else
				printf(RED"write node 0x%04x sensor data to file failed\r\n"NONE,i+1);
		}
		else
			printf(RED"lock or unlock mux failed\r\n"NONE);
	}

	return 0;
}
