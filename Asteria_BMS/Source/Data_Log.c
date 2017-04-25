/*
 * Data_Log.c
 *
 *  Created on: 16-Feb-2017
 *      Author: NIKHIL
 */

#include "Data_Log.h"

FATFS FatFs;
FIL BMS_Log_File;
FRESULT Result;
UINT BytesWritten;

Log_Vars Log_Variables;

char GPS_Date_Time[25];

static char String_Buffer[512];

bool Log_Status = 1;

/* Variable to handle the user buffer index */
static int *String_Index, Memory_Address1 = 0;

/* Variable temp_var is defined just to get any free memory location which is to be assigned to index_counter; otherwise defining pointer
 * without initialization will be dangling pointer */
static uint8_t *Index_Counter,Memory_Address2 = 0;

/* Buffer to store the file name which is created on SD card as soon as logging is started */
char File_Name[50] = "0:/2017-04-21_14-25-50_BMS_3.txt";

uint16_t Stop_Time_Cursor = 0;

uint8_t Create_BMS_Log_File()
{
	uint8_t String_Length = 0;
	uint8_t lcl_counter = 0,Number_in_String[6];
	uint8_t Result = 1;

	String_Index = &Memory_Address1;
	Index_Counter = &Memory_Address2;

	*String_Index = 0;
	memset(String_Buffer,0,sizeof(String_Buffer));

	/* The gps fix flag has to be used in place of 1 */
	if(1)
		sprintf(GPS_Date_Time, "%02d-%02d-%04d %02d-%02d-%02d", 21, 4, 2017,14,25,50);
	else
		sprintf(GPS_Date_Time, "%02d-%02d-%04d %02d-%02d-%02d", 0,0,0,0,0,0);

	/* File name for log file inside the directory */
	File_Name[lcl_counter++] = '0';
	File_Name[lcl_counter++] = ':';
	File_Name[lcl_counter++] = '/';

	/* Fill the file_name buffer with start time and date; Assuming GPS has given the date and time */
	File_Name[lcl_counter++] = GPS_Date_Time[6];
	File_Name[lcl_counter++] = GPS_Date_Time[7];
	File_Name[lcl_counter++] = GPS_Date_Time[8];
	File_Name[lcl_counter++] = GPS_Date_Time[9];
	File_Name[lcl_counter++] = '-';
	File_Name[lcl_counter++] = GPS_Date_Time[3];
	File_Name[lcl_counter++] = GPS_Date_Time[4];
	File_Name[lcl_counter++] = '-';
	File_Name[lcl_counter++] = GPS_Date_Time[0];
	File_Name[lcl_counter++] = GPS_Date_Time[1];
	File_Name[lcl_counter++] = '_';

	for (int index = 0; index < 8; index++)
	{
		File_Name[lcl_counter++] = GPS_Date_Time[11 + index];
	}
	File_Name[lcl_counter++] = '_';
	File_Name[lcl_counter++] = 'B';
	File_Name[lcl_counter++] = 'M';
	File_Name[lcl_counter++] = 'S';
	File_Name[lcl_counter++] = '_';

	/* Assuming this file count is read from internal flash */
	Total_Num_of_Files++;

	/* Convert the number into string */
	itoa(Total_Num_of_Files, (char*) Number_in_String, 10);

	String_Length = strlen((char*) Number_in_String);

	/* Add the file number in file name */
	for (int index = 0; index < String_Length; index++) {
		File_Name[lcl_counter++] = Number_in_String[index];
	}
	File_Name[lcl_counter++] = '.';
	File_Name[lcl_counter++] = 't';
	File_Name[lcl_counter++] = 'x';
	File_Name[lcl_counter++] = 't';
	File_Name[lcl_counter++] = '\0';

	if(f_mount(&FatFs,"0",1) != FR_OK)
	{
		Log_Status = 0;
		Result = 0;
	}
	/* If succeeds in opening the file then start writing data to it; FR_OK: Success, otherwise: Error in opening the file */
	/* This action of opening the file with FA_CREATE_ALWAYS,FA_CREATE_NEW or FA_OPEN_EXISITING depends on Charging/Power up /
	 * Wake up from Sleep etc */
	if (f_open(&BMS_Log_File, File_Name, FA_CREATE_ALWAYS | FA_WRITE | FA_READ)	!= FR_OK)
	{
		Log_Status = 0;
		Result = 0;
	}

	*String_Index += sprintf(String_Buffer,"Asteria BMS %d.%d.%d Log at 30 Hz\r\n", BMS_Firmware_Version[0], BMS_Firmware_Version[1],BMS_Firmware_Version[2]);

	*String_Index += sprintf(&String_Buffer[*String_Index], "UTC Start: ");

	/* Copy the start time value to user data buffer and update its index */
	GPS_Date_Time[13] = ':';
	GPS_Date_Time[16] = ':';

	memcpy(&String_Buffer[*String_Index], GPS_Date_Time, 19);
	*String_Index+= 19;

	/* Record the location at which stop time is to be written on SD card */
	Stop_Time_Cursor = *String_Index + 8;
	*String_Index+= sprintf(&String_Buffer[*String_Index],", Stop:                    \r\n");

	*String_Index += sprintf(&String_Buffer[*String_Index],"GPS_Date,Start_Time,End_Time,C1_Voltage,C2_Voltage,C3_Voltage,C4_Voltage,C5_Voltage,C6_Voltage,");
	*String_Index += sprintf(&String_Buffer[*String_Index],"Pack_Voltage,Pack_Current,Total_Capacity,Capacity_Used,Pack_Cyles_Used,Battery C/D Rate,");
	*String_Index += sprintf(&String_Buffer[*String_Index],"C/D Status,Temperature,Final_Pack_Voltage,Flight_Time,Health_Status_Register\r\n");

	if(f_write(&BMS_Log_File,String_Buffer,*String_Index,&BytesWritten) != FR_OK)
	{
		Result = 0;
	}

	f_sync(&BMS_Log_File);

	*String_Index = 0;
	memset(String_Buffer,0,sizeof(String_Buffer));

	GPS_Date_Time[13] = ':';
	GPS_Date_Time[16] = ':';

	return Result;
}

uint8_t Log_All_Data()
{
	int Int_Values[3];
	float Float_Values[6];
	long Long_Values[4];
	uint8_t Char_Values[1];
	uint8_t Result = 1;

	*String_Index = 0;
	memset(String_Buffer,0,sizeof(String_Buffer));
	f_open(&BMS_Log_File,File_Name,FA_OPEN_EXISTING | FA_WRITE | FA_READ);
	f_lseek(&BMS_Log_File,BMS_Log_File.fsize);

//	int length = 0;

//	if(1)
//		length = sprintf(GPS_Date_Time, "%02d-%02d-%04d,",6,4,2017);
//	else
//		length = sprintf(GPS_Date_Time, "%02d-%02d-%04d,", 0,0,0);

	strncpy(&String_Buffer[*String_Index],GPS_Date_Time,10);
	*String_Index += 10;

	String_Buffer[(*String_Index)++] = ',';

	Long_Values[(*Index_Counter)++] = (Get_System_Time()/1000);								// Start Time
	log_sprintf(Long_Values,String_Buffer,Index_Counter,String_Index,LONG_DATA);

	Long_Values[(*Index_Counter)++] = (Get_System_Time()/1000);								// End Time
	log_sprintf(Long_Values,String_Buffer,Index_Counter,String_Index,LONG_DATA);

	Float_Values[(*Index_Counter)++] = Get_Cell1_Voltage();										// Cell1 Voltage
	Float_Values[(*Index_Counter)++] = Get_Cell2_Voltage();										// Cell2 Voltage
	Float_Values[(*Index_Counter)++] = Get_Cell3_Voltage();										// Cell3 Voltage
	Float_Values[(*Index_Counter)++] = Get_Cell6_Voltage();										// Cell4 Voltage
	Float_Values[(*Index_Counter)++] = Get_Cell7_Voltage();										// Cell5 Voltage
	Float_Values[(*Index_Counter)++] = Get_Cell8_Voltage();										// Cell6 Voltage
	log_sprintf(Float_Values,String_Buffer,Index_Counter,String_Index,SHORT_FLOAT_DATA);

	Float_Values[(*Index_Counter)++] = Get_BMS_Pack_Voltage();										// Pack Voltage
	log_sprintf(Float_Values,String_Buffer,Index_Counter,String_Index,FLOAT_DATA);

	Float_Values[(*Index_Counter)++] = 	(Get_BMS_Pack_Current()/1000);										// Pack Voltage
	Float_Values[(*Index_Counter)++] = (float)BMS_Get_Initial_Capacity();											// Total Capacity
	Float_Values[(*Index_Counter)++] = BMS_Get_Total_Capacity_Used();											// Used Capacity
	log_sprintf(Float_Values,String_Buffer,Index_Counter,String_Index,FLOAT_DATA);

	Int_Values[(*Index_Counter)++] = 5;												// Pack Cycles
	log_sprintf(Int_Values,String_Buffer,Index_Counter,String_Index,SHORT_INT_DATA);

	Int_Values[(*Index_Counter)++] = 800;											// Charge/Discharge Rate
	log_sprintf(Int_Values,String_Buffer,Index_Counter,String_Index,INT_DATA);

	Char_Values[(*Index_Counter)++] = Log_Variables.Charging_Discharging_Status;	// Charge/Discharge Status
	log_sprintf(Char_Values,String_Buffer,Index_Counter,String_Index,CHAR_DATA);

	Float_Values[(*Index_Counter)++] = Get_BMS_Pack_Temperature();										// Temperature of Pack Read By Sensor
	Float_Values[(*Index_Counter)++] = 19.6;										// Final Pack Voltage read after charge/discharge cycle
	log_sprintf(Float_Values,String_Buffer,Index_Counter,String_Index,SHORT_FLOAT_DATA);

	Long_Values[(*Index_Counter)++] = 360;											// Flight Time to be Received From AP
	log_sprintf(Long_Values,String_Buffer,Index_Counter,String_Index,LONG_DATA);

	uint16_t Error_Code = 0x55AA;
//	Int_Values[(*Index_Counter)++] = 0xF0F0;										// To be updated based on status from BMS IC
//	log_sprintf(Int_Values,String_Buffer,Index_Counter,String_Index,INT_DATA);

	for(int i =0 ; i < 16; i++)
	{
		if((Error_Code & 0x8000))
		{
			String_Buffer[(*String_Index)++] = '1';
		}
		else
			String_Buffer[(*String_Index)++] = '0';
		Error_Code <<= 1;
	}

	String_Buffer[(*String_Index)++] = ',';
	String_Buffer[(*String_Index)++] = '\r';
	String_Buffer[(*String_Index)++] = '\n';

	if (f_write(&BMS_Log_File, String_Buffer, *String_Index, &BytesWritten) != FR_OK)
	{
		Log_Status = 0;
		Result = 0;
	}

	Log_Status = 1;

	if(f_sync(&BMS_Log_File) != FR_OK)
		Result = 0;

	return Result;

}

void Stop_Log()
{
	uint8_t length;
	memset(String_Buffer,0,sizeof(String_Buffer));
	f_lseek(&BMS_Log_File,Stop_Time_Cursor);

	if(1)
		length = sprintf(GPS_Date_Time, "%02d-%02d-%04d %02d:%02d:%02d", 6, 4, 2017,16,31,50);
	else
		length = sprintf(GPS_Date_Time, "%02d-%02d-%04d %02d:%02d:%02d", 0,0,0,0,0,0);

	f_write(&BMS_Log_File, GPS_Date_Time, (UINT)length, &BytesWritten);
	f_close(&BMS_Log_File);
}

/* Function to convert the user variables into CSV format
 * Parameters:
 * 		data_array: pointer to the array of integer/float/long/short float/short int
 * 		dst_array : pointer to the array of character to which resulting string is stored
 * 		count 	  : number of data values to be converted into string
 * 		offset    : location from which converted characters are stored
 * 		data_type : it can be INT_DATA/SHORT_INT_DATA/FLOAT_DATA/SHORT_FLOAT_DATA/LONG/CHAR_DATA
 *
 * */
void log_sprintf(void *data_array,char *dst_array,unsigned char *count,int *offset,unsigned char data_type)
{
	unsigned char index = 0;
	int lcl_index = 0 ;

	switch(data_type)
	{
		case INT_DATA:
			data_type = INT_SIZE_;
			for(index = 0;index < *count; index++)
			{
				int data;
				data = ((int*)data_array)[index];

				if(data >=0)
					dst_array[*offset] = '+';
				else
				{
					dst_array[*offset] = '-';
					data = -data;
				}

				if(data > MAX_INT_VALUE)
					data = MAX_INT_VALUE;

				while(data != 0 && lcl_index < (data_type - 1))
				{
					dst_array[*offset + data_type - ++lcl_index] = (data % 10) + '0';
					data = data / 10;
				}
				while(lcl_index != (data_type-1))
					dst_array[*offset + data_type - ++lcl_index] = '0';

				dst_array[*offset + data_type] = ',';
				*offset += (data_type + 1);
				lcl_index = 0;
			}
			break;


		case LONG_DATA:
			data_type = LONG_SIZE_;
			for(index = 0;index < *count; index++)
			{
				long data;
				data = ((long*)data_array)[index];

				if(data >=0 )
					dst_array[*offset] = '+';
				else
				{
					dst_array[*offset] = '-';
					data = -data;
				}

				if(data > MAX_LONG_VALUE)
					data = MAX_LONG_VALUE;

				while(data != 0 && lcl_index < (data_type-1))
				{
					dst_array[*offset + data_type - ++lcl_index] = (data % 10) + '0';
					data = data / 10;
				}
				while(lcl_index != (data_type-1))
					dst_array[*offset + data_type - ++lcl_index] = '0';

				dst_array[*offset + data_type] = ',';
				*offset += (data_type + 1);
				lcl_index = 0;
			}
			break;
	case SHORT_INT_DATA:
			data_type = SHORT_INT_SIZE_;
			for(index = 0;index < *count; index++)
			{
				int data;
				data = ((int*)data_array)[index];

				if(data >=0 )
					dst_array[*offset] = '+';
				else
				{
					dst_array[*offset] = '-';
					data = -data;
				}

				if(data > MAX_SHORT_INT_VALUE)
					data = MAX_SHORT_INT_VALUE;

				while(data != 0 && lcl_index < data_type - 1)
				{
					dst_array[*offset + data_type - ++lcl_index] = (data % 10) + '0';
					data = data / 10;
				}

				while(lcl_index != (data_type-1))
					dst_array[*offset + data_type - ++lcl_index] = '0';

				dst_array[*offset + data_type] = ',';
				*offset += (data_type + 1);
				lcl_index = 0;
			}
			break;

	case SHORT_FLOAT_DATA:
			data_type = SHORT_FLOAT_SIZE_;
			for(index = 0;index < *count; index++)
			{
				float data;
				int lcl_int = 0;
				data = ((float*)data_array)[index];
				if(data >=0)
					dst_array[*offset] = '+';
				else
				{
					dst_array[*offset] = '-';
					data = -data;
				}

				if(data > MAX_SHORT_FLOAT_VALUE)
					data = MAX_SHORT_FLOAT_VALUE;

					data *= 100;
					lcl_int = (int)data;

				while(lcl_int != 0 && lcl_index < data_type - 1)
				{
					if(lcl_index == DECIMAL_POINT_PLACE)
						dst_array[*offset + data_type - ++lcl_index] = '.';
					else
					{
						dst_array[*offset + data_type - ++lcl_index] = (lcl_int % 10) + '0';
						lcl_int = lcl_int / 10;
					}

				}
				while(lcl_index != (data_type-1))
				{
					if(lcl_index == DECIMAL_POINT_PLACE)
						dst_array[*offset + data_type - ++lcl_index] = '.';
					else
						dst_array[*offset + data_type - ++lcl_index] = '0';
				}

				dst_array[*offset + data_type] = ',';
				*offset += (data_type + 1);
				lcl_index = 0;
			}
			break;
	case FLOAT_DATA:
			data_type = FLOAT_SIZE_;

			for(index = 0;index < *count; index++)
			{
				float data;
				int lcl_int = 0;
				data = ((float*)data_array)[index];
				if(data >=0)
					dst_array[*offset] = '+';
				else
				{
					dst_array[*offset] = '-';
					data = -data;
				}

				if(data > MAX_FLOAT_VALUE)
					data = MAX_FLOAT_VALUE;

					data *= 100;
					lcl_int = (int)data;

				while(lcl_int != 0 && lcl_index < data_type - 1)
				{
					if(lcl_index == DECIMAL_POINT_PLACE)
						dst_array[*offset + data_type - ++lcl_index] = '.';
					else
					{
						dst_array[*offset + data_type - ++lcl_index] = (lcl_int % 10) + '0';
						lcl_int = lcl_int / 10;
					}
				}
				while(lcl_index != (data_type-1))
				{
					if(lcl_index == DECIMAL_POINT_PLACE)
						dst_array[*offset + data_type - ++lcl_index] = '.';
					else
						dst_array[*offset + data_type - ++lcl_index] = '0';
				}

				dst_array[*offset + data_type] = ',';
				*offset += (data_type + 1);
				lcl_index = 0;
			}
			break;
	case CHAR_DATA:
		data_type = CHAR_SIZE_;
		for(index = 0;index < *count; index++)
		{
			signed char data;
			data = ((signed char*)data_array)[index];
			if(data >=0)
				dst_array[*offset] = '+';
			else
			{
				dst_array[*offset] = '-';
				data = -data;
			}
			if(data > MAX_CHAR_VALUE)
				data = MAX_CHAR_VALUE;

			if(data == 0)
			{
				dst_array[*offset + data_type - ++lcl_index] = (data % 10) + '0';
			}
			else
			{
				while (data != 0 && lcl_index < data_type - 1)
				{
					dst_array[*offset + data_type - ++lcl_index] = (data % 10)+ '0';
					data /= 10;
				}
			}

			dst_array[*offset + data_type] = ',';
			*offset += (data_type + 1);
			lcl_index = 0;
		}

			break;
	}
	*count = 0;
}

