/*
 * Data_Log.h
 *
 *  Created on: 16-Feb-2017
 *      Author: NIKHIL
 */

#ifndef DATA_LOG_H_
#define DATA_LOG_H_

#include "TIMER_API.h"
#include "ff.h"
#include "GPIO_API.h"
#include "BMS_Communication.h"
#include "FLASH_API.h"

#define DIRECTORY_NAME_START_ADDRESS				ADDR_FLASH_PAGE_127
#define DIRECTORY_NAME_END_ADDRESS					ADDR_FLASH_PAGE_127+8

/* Maximum file count in each directory */
#define MAX_FILE_COUNT

/* Base number for converting the integer to strings */
#define MAX_DIGITS_IN_COUNT							6
#define DECIMAL_BASE								10
#define BINARY_BASE									2
#define OCTAL_BASE									85

/* Maximum ranges of the variables for different data types */
#define MAX_CHAR_VALUE								9
#define MAX_INT_VALUE								99999
#define MAX_SHORT_INT_VALUE							999
#define MAX_LONG_VALUE								999999999
#define MAX_FLOAT_VALUE								9999.99
#define MAX_SHORT_FLOAT_VALUE						99.99

#define DECIMAL_POINT_PLACE							2

enum SD_Status
{
	SDC_NOT_PRESENT = 0,SDC_PRESENT
};

/* Different data types which are getting used to log them on SD card */
enum data_types
{
	CHAR_DATA = 0,SHORT_INT_DATA,INT_DATA,SHORT_FLOAT_DATA,FLOAT_DATA,LONG_DATA
};

/* Sizes(number converted to characters) of the different data types */
enum data_sizes
{
	CHAR_SIZE_ = 2,SHORT_INT_SIZE_ = 4,INT_SIZE_ = 6,FLOAT_SIZE_ = 8,SHORT_FLOAT_SIZE_ = 6,LONG_SIZE_ = 10
};

typedef struct
{
	uint32_t Start_Time;
	uint32_t End_Time;
	float C1_Voltage;
	float C2_Voltage;
	float C3_Voltage;
	float C4_Voltage;
	float C5_Voltage;
	float C6_Voltage;

	float Pack_Voltage;
	uint16_t Total_Capacity;
	uint16_t Capacity_Used;
	uint16_t Pack_Cycles;
	uint8_t Charging_Discharging_Status;

	float Temperature;
	float Final_Pack_Voltage;

	uint32_t Flight_Time;
	uint32_t Health_Status_Register;
	char GPS_Date[10];
	char Battery_Charge_Discharge_Date[10];


}Log_Vars;

extern Log_Vars Log_Variables;
extern const uint8_t BMS_Firmware_Version[3];

//extern Log_SD_Summary_Vars SD_Summary_Data;
extern bool Log_Status;

uint16_t Total_Num_of_Files;

/* Variable which returns the status as 0 if anything goes wrong in log related functions */
extern uint8_t Check_SD_Card;

uint8_t Create_BMS_Log_File();
void log_sprintf(void *data_array,char *dst_array,unsigned char *count,int *offset,unsigned char data_type);
uint8_t Log_All_Data();

#endif /* DATA_LOG_H_ */
