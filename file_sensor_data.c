#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_sensor_data.h"


sensor_data_save_t sensor_data_save;


sensor_data_save_t* readSensorData(unsigned short DstAddr)
{
	FILE *fp;
	sensor_data_save_t *sensor_data;

	static char read_str[100];
	static char compare_str[20];
	static short temp_short;

	sensor_data = (sensor_data_save_t *)malloc(sizeof(sensor_data_save_t) );
	if( sensor_data == NULL )
	{
		return NULL;
	}
	memset(sensor_data, 0, sizeof(sensor_data_save_t));
	if( (fp= fopen(FILE_NAME, "r") ) == NULL)
	{
		printf("open file failed\r\n");
		return NULL;
	}
	sprintf(compare_str, "LOCK_No.0x%04x", DstAddr);
	while( fgets( read_str, 100, fp) )
	{
//		printf("%s", read_str);
		if( !memcmp( compare_str, read_str, strlen(compare_str)) )
		{
			printf("found it\r\n");
			fseek(fp, strlen(" x = "), SEEK_CUR);
			fgets( read_str, 10, fp );
			temp_short = (short) atol(read_str);
			printf("X:str = %s, integer = %d\r\n", read_str, temp_short);
			sensor_data->x = temp_short;

			fseek(fp, strlen(" y = "), SEEK_CUR);
			fgets( read_str, 10, fp );
			temp_short = (short) atol(read_str);
			printf("Y:str = %s, integer = %d\r\n", read_str, temp_short);
			sensor_data->y = temp_short;

			fseek(fp, strlen(" z = "), SEEK_CUR);
			fgets( read_str, 10, fp );
			temp_short = (short) atol(read_str);
			printf("Z:str = %s, integer = %d\r\n", read_str, temp_short);
			sensor_data->z = temp_short;

			fseek(fp, strlen(" threshold = "), SEEK_CUR);
			fgets( read_str, 10, fp );
			temp_short = (short) atol(read_str);
			printf("Threshold:str = %s, integer = %d\r\n", read_str, temp_short);
			sensor_data->threshold = temp_short;
			fclose(fp);
			return sensor_data;
		}	
	}
	fclose(fp);
	return NULL;
}

int writeSensorData(unsigned short DstAddr, sensor_data_save_t *sensor_data)
{
	FILE *fp;
	static char write_str[100];

	if( (fp = fopen(FILE_NAME, "a+")) == NULL )
	{
		printf("open file failed.\r\n");
		return -1;
	}
	sprintf(write_str, "LOCK_No.0x%04x", DstAddr);
	fputs( write_str, fp );
	fputs( "\n", fp );
	sprintf( write_str, " x = %d", sensor_data->x );
	fputs( write_str, fp );
	fputs( "\n", fp );
	sprintf( write_str, " y = %d", sensor_data->y );
	fputs( write_str, fp );
	fputs( "\n", fp );
	sprintf( write_str, " z = %d", sensor_data->z );
	fputs( write_str, fp );
	fputs( "\n", fp );
	sprintf( write_str, " threshold = %d", sensor_data->threshold );
	fputs( write_str, fp );
	fputs( "\n", fp );
	fputs( "\n", fp );
	fclose(fp);
	return 0;
}

