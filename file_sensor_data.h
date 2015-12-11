#ifndef __FILE_SENSOR_DATA__
#define __FILE_SENSOR_DATA__

#define FILE_NAME "/opt/sensor.data"

typedef struct {
	short x;
	short y;
	short z;
	short threshold;
} sensor_data_save_t;

extern sensor_data_save_t sensor_data_save;
int writeSensorData(unsigned short DstAddr, sensor_data_save_t *sensor_data);
sensor_data_save_t* readSensorData(unsigned short DstAddr);

#endif

