/*
 * BMS_Data_Log.h
 *
 *  Created on: 16-Feb-2017
 *      Author: NIKHIL
 */

#ifndef BMS_DATA_LOG_H_
#define BMS_DATA_LOG_H_

#include "ff.h"

/* Base number for converting the integer to strings */
#define MAX_DIGITS_IN_COUNT							6		/* Specifies the maximum digits in decimal number */
#define DECIMAL_BASE								10
#define BINARY_BASE									2
#define OCTAL_BASE									8

/* Maximum ranges of the variables for different data types */
#define MAX_CHAR_VALUE								9
#define MAX_INT_VALUE								99999
#define MAX_SHORT_INT_VALUE							999
#define MAX_LONG_VALUE								999999999
#define MAX_FLOAT_VALUE								99999.999
#define MAX_SHORT_FLOAT_VALUE						99.99

/* Specifies the resolution for float value which can be .99(for 2) or .999(for 3) */
#define SF_DECIMAL_POINT_PLACE						2
#define LF_DECIMAL_POINT_PLACE						3

/* Enums for SD card status
 * @SDC_NOT_PRESENT	: SD card is not present in the slot
 * @SDC_PRESENT		: SD card is present in the slot */
enum SD_Status
{
	SDC_NOT_PRESENT = 0,SDC_PRESENT
};

/* Different data types which are getting used to log them on SD card */
enum data_types
{
	CHAR_DATA = 0,SHORT_INT_DATA,INT_DATA,SHORT_FLOAT_DATA,FLOAT_DATA,LONG_DATA
};

/* Sizes(decimal number converted to characters) of the different data types */
enum data_sizes
{
	CHAR_SIZE_ = 2,SHORT_INT_SIZE_ = 4,INT_SIZE_ = 6,FLOAT_SIZE_ = 10,SHORT_FLOAT_SIZE_ = 7,LONG_SIZE_ = 10
};

typedef struct
{
	uint32_t Start_Time;
	uint32_t End_Time;
	float Final_Pack_Voltage;

	uint32_t Flight_Time;
	uint32_t Health_Status_Register;
	char GPS_Date[10];
	char Battery_Charge_Discharge_Date[10];
}Log_Vars;

extern const uint8_t BMS_Firmware_Version[3];

uint16_t Total_Num_of_Files;

/* Function prototypes */
uint8_t Create_BMS_Log_File();
void log_sprintf(void *data_array,char *dst_array,uint8_t *count,uint32_t *offset,uint8_t data_type);
uint8_t Log_All_Data();
void Stop_Log();

#endif /* BMS_DATA_LOG_H_ */
