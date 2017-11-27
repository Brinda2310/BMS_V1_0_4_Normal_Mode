/*
 * BMS_Data_Log.c
 *
 *  Created on: 16-Feb-2017
 *      Author: NIKHIL
 */

#include <BMS_Data_Log.h>

/* Global variables for FATFS file operations */
FATFS FatFs;
FIL Summary_File;
FIL BMS_Log_File;
FRESULT Result;
UINT BytesWritten;

/* Variable to update the time and date received from GPS */
static char GPS_Date_Time[25];

/* Character buffer to hold the variables to be written to the SD card */
static char String_Buffer[4096];

/* Variable to handle the buffer index for data to be written to the SD card */
static uint32_t *String_Index, Memory_Address1 = 0;

/* Variable temp_var is defined just to get any free memory location which is to be assigned to index_counter; otherwise defining pointer
 * without initialization will be dangling pointer */
static uint8_t *Index_Counter,Memory_Address2 = 0;

/* Buffer to store the file name which is created on SD card as soon as logging is started. Right now
 * this name is hard coded, later it will be changed as per the log summary file */
static char File_Name[50] = "";

/* This variable holds the cursor position for stop time to be written to SD card */
uint16_t Stop_Time_Cursor = 0;

/* Variable to hold the respective values as mentioned in the name and the same variables which are updated in log_summary_file */
Log_SD_Summary_Vars SD_Summary_Data;

/* Variable to compare the current read power up number from SD card and the one which code is running at */
static uint16_t Old_Power_Up_Num = 0;

/* New file created only upon power up otherwise logging is done in the old file after wake up */
static bool New_File_Created = false;

/* Variable to create only one log file in each power up */
static bool Power_Up_BMS = false;

/* Variable to log the count for number of restarts occurred for ISL */
uint32_t ASIC_Restart_Count = 0;

/**
 * @brief  Function to create/check the log summary file. Create the BMS log files by reading the counts
 * 		   stored in the summary file
 * @param  None
 * @retval RESULT_OK	: Log initialization is successful
 * 		   RESULT_ERROR	: Log initialization is unsuccessful
 */
uint8_t BMS_Log_Init()
{
	uint8_t Result = RESULT_OK;

	/* Closing of the file handles is necessary as code may return error in case it is not able to open the files */
	f_close(&BMS_Log_File);
	f_close(&Summary_File);

	/* We want to create only one log file during each power up. This flag does that job. It becomes false
	 * as soon as file is created */
	if(Power_Up_BMS == false)
	{
		Power_Up_BMS = true;
	}

	if(Power_Up_BMS == true)
	{
		/* Create the log summary file in SD card. If it already exists then get the counts for total number of
		 * files and power up number and create the new log file by incrementing the total file number */
		if(Create_Log_Summary_File() == RESULT_OK)
		{
			Result = Create_BMS_Log_File();
		}
		else
		{
			Result = RESULT_ERROR;
		}
	}

	return Result;
}

/**
 * @brief  Function to create the log summary file and write strings for number of directories,
 * 		   current directory and file counts along with header data.
 * @retval RESULT_OK	: Log summary file creation is successful
 *		   RESULT_ERROR	: Log summary file creation is unsuccessful
 */
uint8_t Create_Log_Summary_File()
{
	FRESULT Result;

	/* Very first thing that should happen is checking the log summary file and initialize the variables
	 * used for indexing */
	String_Index = &Memory_Address1;
	Index_Counter = &Memory_Address2;

	/* Mount the drive; If there is an error while mounting the SD card return the error status
	 * to the caller function*/
	Result = f_mount(&FatFs, "0", 1);
	if(Result != FR_OK)
	{
		return RESULT_ERROR;
	}

	/* This will create the Log_Summary_File if it is not created previously; This will execute only
	 * once in lifetime unless someone delete the Log_Summary_File */
	Result = f_open(&Summary_File, "0:/Log_Summary_File.txt",FA_OPEN_EXISTING| FA_WRITE | FA_READ);

	if(Result == FR_NO_FILE)
	{
		uint8_t Total_Num_Files_String[] = "Total_Number_of_Files:0     *";
		uint8_t Power_Up_Num_String[] = "Power Up Number:0     *";

		/* If log summary file does not exist then create one and write the strings to the file */
		if (f_open(&Summary_File, "0:/Log_Summary_File.txt",FA_CREATE_ALWAYS| FA_WRITE | FA_READ) == FR_OK)
		{
			SD_Summary_Data.Total_Num_of_Files = 0;
			if (f_write(&Summary_File, Total_Num_Files_String, sizeof(Total_Num_Files_String), &BytesWritten)!= FR_OK) // 30
			{
				return RESULT_ERROR;
			}
			if (f_write(&Summary_File, Power_Up_Num_String, sizeof(Power_Up_Num_String), &BytesWritten)!= FR_OK)	// 24
			{
				return RESULT_ERROR;
			}

			/* Write the data stored in buffers to the SD card */
			f_sync(&Summary_File);

			Old_Power_Up_Num = SD_Summary_Data.Power_Up_Number;

			SD_Summary_Data.Power_Up_Number++;

			/* Update the Power Up index in Log_Summary_File */
			Write_Count_Log_Summary_File(POWER_UP_NUMBER_INDEX,SD_Summary_Data.Power_Up_Number);

		}
		return RESULT_OK;
	}

	uint8_t SD_Result = RESULT_OK;

	/* Retrieve all the counts from Log_Summary_File */
	SD_Result = Get_Count_Log_Summary_File(POWER_UP_NUMBER_INDEX, &SD_Summary_Data.Power_Up_Number);
	SD_Result = Get_Count_Log_Summary_File(TOTAL_FILE_COUNT_INDEX, &SD_Summary_Data.Total_Num_of_Files);

	/* If MCU is rebooted or Powered up again then update the counts in the Log_Summary_File */
	if((Old_Power_Up_Num != SD_Summary_Data.Power_Up_Number) && New_File_Created == false)
	{
		Old_Power_Up_Num = SD_Summary_Data.Power_Up_Number;
		SD_Summary_Data.Power_Up_Number++;

		/* Store the total character counts that are there in the file which is useful to write at the respective cursor position */
		SD_Result = Write_Count_Log_Summary_File(POWER_UP_NUMBER_INDEX,SD_Summary_Data.Power_Up_Number);
	}

	/* Clear the buffer indices so that buffers can be filled with new data from start */
	*String_Index = 0;
	*Index_Counter= 0;

//	char Buffer[10],Len;
//	Len = sprintf(Buffer,"%d\r",SD_Summary_Data.Power_Up_Number);
//	BMS_Debug_COM_Write_Data(Buffer,Len);
	return SD_Result;
}

/**
 * @brief  Function to retrieve the different counts from log_summary_file
 * 		   current directory and file counts along with header data.
 * @param  Offset		: The cursor position from which data is to read from file
 * @param  Variable		: The pointer to the variable in which read data from file is stored
 * @retval RESULT_OK	: Counts reading from log summary file is successful
 *		   RESULT_ERROR	: Counts reading from log summary file is unsuccessful
 */
uint8_t Get_Count_Log_Summary_File(uint32_t Offset,uint16_t *Variable)
{
	char Rx_Data[MAX_DIGITS_IN_COUNT];

	/* Take the cursor in file to the position mentioned */
	f_lseek(&Summary_File,Offset);

	/* Read the required count from the Log_Summary_File */
	if(f_read(&Summary_File, Rx_Data, MAX_DIGITS_IN_COUNT, &BytesWritten) == FR_OK)
	{
		*Variable = atoi(Rx_Data);
		return RESULT_OK;
	}
	else
	{
		return RESULT_ERROR;
	}
}

/**
 * @brief  Function to create the new BMS_Log_File on SD card with incremented file number,
 * 		   current directory and file counts along with header data.
 * @retval RESULT_OK	: Log file creation is successful
 *		   RESULT_ERROR	: Log file creation is unsuccessful
 */
uint8_t Create_BMS_Log_File()
{
	uint8_t Result = RESULT_OK;
	uint8_t Lcl_Counter = 0,String_Length = 0, Number_in_String[6];

	/* Allocate the valid memory addresses to the pointers */
	String_Index = &Memory_Address1;
	Index_Counter = &Memory_Address2;

	/* Clear the buffers and reset the buffer index before using them to store new data */
	*String_Index = 0;
	memset(String_Buffer,0,sizeof(String_Buffer));

	/* The gps fix flag has to be used in place of 1 */
	if(1)
		sprintf(GPS_Date_Time, "%02d-%02d-%04d %02d-%02d-%02d", 22,11,2017,15,00,05);
	else
		sprintf(GPS_Date_Time, "%02d-%02d-%04d %02d-%02d-%02d", 0,0,0,0,0,0);

	/* Make sure that only one is file created on each power up */
	if (Power_Up_BMS == true && New_File_Created == false)
	{
		/* File name for log file inside the directory */
		File_Name[Lcl_Counter++] = '0';
		File_Name[Lcl_Counter++] = ':';
		File_Name[Lcl_Counter++] = '/';

		/* Fill the File_Name buffer with start time and date */
		File_Name[Lcl_Counter++] = GPS_Date_Time[6];
		File_Name[Lcl_Counter++] = GPS_Date_Time[7];
		File_Name[Lcl_Counter++] = GPS_Date_Time[8];
		File_Name[Lcl_Counter++] = GPS_Date_Time[9];
		File_Name[Lcl_Counter++] = '-';
		File_Name[Lcl_Counter++] = GPS_Date_Time[3];
		File_Name[Lcl_Counter++] = GPS_Date_Time[4];
		File_Name[Lcl_Counter++] = '-';
		File_Name[Lcl_Counter++] = GPS_Date_Time[0];
		File_Name[Lcl_Counter++] = GPS_Date_Time[1];
		File_Name[Lcl_Counter++] = '_';

		for (int index = 0; index < 8; index++)
		{
			File_Name[Lcl_Counter++] = GPS_Date_Time[11 + index];
		}

		File_Name[Lcl_Counter++] = '_';
		File_Name[Lcl_Counter++] = 'B';
		File_Name[Lcl_Counter++] = 'M';
		File_Name[Lcl_Counter++] = 'S';
		File_Name[Lcl_Counter++] = '_';

		/* Increment the file number */
		SD_Summary_Data.Total_Num_of_Files++;

		/* Store the update file count number in the Log_Summary_File so that next time
		 * new file will be created by reading the file count value */
		Write_Count_Log_Summary_File(TOTAL_FILE_COUNT_INDEX,SD_Summary_Data.Total_Num_of_Files);

		/* Convert the number into string */
		itoa(SD_Summary_Data.Total_Num_of_Files, (char*) Number_in_String, 10);
		String_Length = strlen((char*) Number_in_String);

		/* Add the file number in file name */
		for (int index = 0; index < String_Length; index++)
		{
			File_Name[Lcl_Counter++] = Number_in_String[index];
		}
		File_Name[Lcl_Counter++] = '.';
		File_Name[Lcl_Counter++] = 't';
		File_Name[Lcl_Counter++] = 'x';
		File_Name[Lcl_Counter++] = 't';
		File_Name[Lcl_Counter++] = '\0';

		/* Make sure file system is mounted before opening/writing the files */
		if (f_mount(&FatFs, "0", 1) != FR_OK)
		{
			Result = RESULT_ERROR;
		}
		/* If succeeds in opening the file then start writing data to it; FR_OK: Success, otherwise: Error in opening the file */
		/* This action of opening the file with FA_CREATE_ALWAYS,FA_CREATE_NEW or FA_OPEN_EXISITING depends on Charging/Power up /
		 * Wake up from Sleep etc */
		if (f_open(&BMS_Log_File, File_Name,FA_CREATE_ALWAYS | FA_WRITE | FA_READ) != FR_OK)
		{
			Result = RESULT_ERROR;
		}

		/* Start filling the header data in buffer and once done write it to the BMS_Log_File */
		*String_Index += sprintf(String_Buffer,"Asteria BMS %d.%d.%d Log at %d Hz\r\n",BMS_Firmware_Version[0], BMS_Firmware_Version[1],
				BMS_Firmware_Version[2],(uint8_t)_1_SECONDS);

		*String_Index += sprintf(&String_Buffer[*String_Index], "UTC Start: ");

		/* Copy the start time value to user data buffer and update its index */
		GPS_Date_Time[13] = ':';
		GPS_Date_Time[16] = ':';

		memcpy(&String_Buffer[*String_Index], GPS_Date_Time, 19);
		*String_Index += 19;

		/* Record the location at which stop time is to be written on SD card */
		Stop_Time_Cursor = *String_Index + 8;
		*String_Index += sprintf(&String_Buffer[*String_Index],", Stop:                    \r\n");


		/* Fill the buffer with battery parameters */
		*String_Index += sprintf(&String_Buffer[*String_Index], "Battery ID:%s   ",Battery_ID);

		if(BATTERY_TYPE == LI_POLYMER)
		{
			*String_Index += sprintf(&String_Buffer[*String_Index], "Battery Type: Li-Polymer   ");
		}
		else if(BATTERY_TYPE == LI_ION)
		{
			*String_Index += sprintf(&String_Buffer[*String_Index], "Battery Type: Li-Ion   ");
		}
		else
		{
			*String_Index += sprintf(&String_Buffer[*String_Index], "Battery Type: Not defined   ");
		}

		*String_Index += sprintf(&String_Buffer[*String_Index], "Battery Number of Cells:%d   ",(int)BATT_NUMBER_OF_CELLS);
		*String_Index += sprintf(&String_Buffer[*String_Index], "Battery Cpacity(mAH):%0.2f   ",(float)BATT_MAH);
		*String_Index += sprintf(&String_Buffer[*String_Index], "Battery Cell Max(V):%0.2f   ",(float)BATT_CELL_VOLT_MAX);
		*String_Index += sprintf(&String_Buffer[*String_Index], "Battery Cell Min(V):%0.2f   ",(float)BATT_CELL_VOLT_MIN);
		*String_Index += sprintf(&String_Buffer[*String_Index], "Battery Pack Cycles(V):%d   \r\n",(int)BATT_MAX_PACK_CYCLES);

		*String_Index += sprintf(&String_Buffer[*String_Index],"GPS_Date,Start_Time,End_Time,C1_Volt,C2_Volt,C3_Volt,C4_Volt,C5_Volt,C6_Volt,");
		*String_Index += sprintf(&String_Buffer[*String_Index],"Pack_Voltage,Accumulated_Pack_Voltage,Pack_Current,Pack_Current_Adjusted,Total_Capacity,Capacity_Remaining,");
		*String_Index += sprintf(&String_Buffer[*String_Index],"Capacity_Used,Pack_Cyles_Used,Current_Gain,Battery_C_D_Rate,mAH_IN_OUT,C_D_Status,Temperature,");
		*String_Index += sprintf(&String_Buffer[*String_Index],"Final_Pack_Voltage,Flight_Time,Health_Error_Status,I2C_Error_Status,Loop_Rate,ISL_Restart_Count,");
		*String_Index += sprintf(&String_Buffer[*String_Index],"Watchdog_Flag,AP_Status,MCU_Reset_Source\r\n");

//		while((*String_Index) != 1021)
//		{
//			String_Buffer[(*String_Index)++] = 'A';
//		}
//
//		String_Buffer[(*String_Index)++] = ',';
//		String_Buffer[(*String_Index)++] = '\r';
//		String_Buffer[(*String_Index)++] = '\n';

		if (f_write(&BMS_Log_File, String_Buffer, *String_Index, &BytesWritten) != FR_OK)
		{
			Result = RESULT_ERROR;
		}

		f_sync(&BMS_Log_File);
		New_File_Created = true;
	}
	else
	{
		/* If succeeds in opening the file then start writing data to it; FR_OK: Success, otherwise: Error
		 * in opening the file */
		/* This action of opening the file with FA_CREATE_ALWAYS,FA_CREATE_NEW or FA_OPEN_EXISITING depends
		 * on Charging/Power up/Wake up from Sleep etc */
		if (f_open(&BMS_Log_File, File_Name,FA_OPEN_EXISTING | FA_WRITE | FA_READ) != FR_OK)
		{
			Result = RESULT_ERROR;
		}

		f_lseek(&BMS_Log_File,BMS_Log_File.fsize);
	}
	*String_Index = 0;
	memset(String_Buffer,0,sizeof(String_Buffer));

	GPS_Date_Time[13] = ':';
	GPS_Date_Time[16] = ':';

	return Result;
}

/**
 * @brief  Function to write all the BMS variables to the SD card
 * 		   current directory and file counts along with header data.
 * @retval RESULT_OK	: File writing is successful
 *		   RESULT_ERROR	: File writing is unsuccessful
 */
uint8_t Log_All_Data()
{
	int Int_Values[3];
	float Float_Values[6];
	long Long_Values[4];
	uint8_t Char_Values[5];
	uint8_t Result = RESULT_OK;

	/* If the size of file exceeds the 200MB size then initialize all the variables responsible for creating new file*/
	if((Get_BMS_Log_File_Size()/_1MB_FILE_SIZE) > _200MB_FILE_SIZE)
	{
		New_File_Created = false;
		BMS_Log_Init();
	}
	*String_Index = 0;
	memset(String_Buffer,0,sizeof(String_Buffer));

	strncpy(&String_Buffer[*String_Index],GPS_Date_Time,10);
	*String_Index += 10;

	String_Buffer[(*String_Index)++] = ',';

	Long_Values[(*Index_Counter)++] = Get_System_Time_Millis();								// Start Time
	log_sprintf(Long_Values,String_Buffer,Index_Counter,String_Index,LONG_DATA);

	Long_Values[(*Index_Counter)++] = Get_System_Time_Millis();								// End Time
	log_sprintf(Long_Values,String_Buffer,Index_Counter,String_Index,LONG_DATA);

	Float_Values[(*Index_Counter)++] = Get_Cell1_Voltage();									// Cell1 Voltage
	Float_Values[(*Index_Counter)++] = Get_Cell2_Voltage();									// Cell2 Voltage
	Float_Values[(*Index_Counter)++] = Get_Cell3_Voltage();									// Cell3 Voltage
	Float_Values[(*Index_Counter)++] = Get_Cell6_Voltage();									// Cell4 Voltage
	Float_Values[(*Index_Counter)++] = Get_Cell7_Voltage();									// Cell5 Voltage
	Float_Values[(*Index_Counter)++] = Get_Cell8_Voltage();									// Cell6 Voltage
	log_sprintf(Float_Values,String_Buffer,Index_Counter,String_Index,SHORT_FLOAT_DATA);

	Float_Values[(*Index_Counter)++] = Get_BMS_Pack_Voltage();								// Pack Voltage from ISL
	Float_Values[(*Index_Counter)++] = Get_BMS_Accumulated_Pack_Voltage();					// Acculumated Pack Voltage
	log_sprintf(Float_Values,String_Buffer,Index_Counter,String_Index,FLOAT_DATA);

	Float_Values[(*Index_Counter)++] = 	Get_BMS_Pack_Current();								// Pack Current from ISL
	Float_Values[(*Index_Counter)++] = 	Get_BMS_Pack_Current_Adj();							// Accurate Pack Current
	Float_Values[(*Index_Counter)++] = (float)BATTERY_CAPACITY;								// Total Pack Capacity
	Float_Values[(*Index_Counter)++] = (float)Get_BMS_Capacity_Remaining();					// Pack Capacity Remaining
	Float_Values[(*Index_Counter)++] = Get_BMS_Capacity_Used();								// Used Capacity
	log_sprintf(Float_Values,String_Buffer,Index_Counter,String_Index,FLOAT_DATA);

	Int_Values[(*Index_Counter)++] = Get_BMS_Total_Pack_Cycles();														// Pack Cycles
	Int_Values[(*Index_Counter)++] = Current_Gain;
	log_sprintf(Int_Values,String_Buffer,Index_Counter,String_Index,SHORT_INT_DATA);

	Float_Values[(*Index_Counter)++] = Get_BMS_Charge_Discharge_Rate();						// C/D rate in mAH
	Float_Values[(*Index_Counter)++] = C_D_Accumulated_mAH;									// Charge/Discharge mAH
	log_sprintf(Float_Values,String_Buffer,Index_Counter,String_Index,FLOAT_DATA);

	Char_Values[(*Index_Counter)++] = Get_BMS_Charge_Discharge_Status();					// Charge/Discharge Status
	log_sprintf(Char_Values,String_Buffer,Index_Counter,String_Index,CHAR_DATA);

	Float_Values[(*Index_Counter)++] = Get_BMS_Pack_Temperature();							// Pack Temperature
	Float_Values[(*Index_Counter)++] = Get_BMS_Pack_Voltage();								// Final Pack Voltage read after charge/discharge cycle
	log_sprintf(Float_Values,String_Buffer,Index_Counter,String_Index,SHORT_FLOAT_DATA);

	Long_Values[(*Index_Counter)++] = 360;													// Flight Time
	log_sprintf(Long_Values,String_Buffer,Index_Counter,String_Index,LONG_DATA);

	uint32_t Error_Code = Error_Check_Data;
	uint16_t Health_Data_Start_Index = *String_Index;

	/* Logic to convert the decimal value to binary and storing the same in buffer */
	for(int i =0 ; i < 32; i++)
	{
		if((Error_Code & 0x80000000))
		{
			String_Buffer[(*String_Index)++] = '1';
		}
		else
			String_Buffer[(*String_Index)++] = '0';
		Error_Code <<= 1;
	}

	memcpy(&BMS_Data.Health_Status_Info[0],&String_Buffer[Health_Data_Start_Index],32);
	String_Buffer[(*String_Index)++] = ',';

	/* These are the flags which are set and reset based on I2C operations */
	uint32_t *I2C_Error_Code = (uint32_t*)&I2C_Error_Flag;

	uint16_t I2C_Error_Start_Index = *String_Index;

	for(int i =0 ; i < 32; i++)
	{
		if((*I2C_Error_Code & 0x80000000))
		{
			String_Buffer[(*String_Index)++] = '1';
		}
		else
			String_Buffer[(*String_Index)++] = '0';
		*I2C_Error_Code <<= 1;
	}

	memcpy(BMS_Data.I2C_Error_Info,&String_Buffer[I2C_Error_Start_Index],32);
	String_Buffer[(*String_Index)++] = ',';

	Int_Values[(*Index_Counter)++] = Loop_Rate_Log_Counter;
	log_sprintf(Int_Values,String_Buffer,Index_Counter,String_Index,SHORT_INT_DATA);

	Int_Values[(*Index_Counter)++] = ASIC_Restart_Count;
	log_sprintf(Int_Values,String_Buffer,Index_Counter,String_Index,INT_DATA);

	Char_Values[(*Index_Counter)++] = BMS_Watchdog_Enable;
	Char_Values[(*Index_Counter)++] = AP_Stat_Data.bytes[0];
	Char_Values[(*Index_Counter)++] = Reset_Source;
	log_sprintf(Char_Values,String_Buffer,Index_Counter,String_Index,CHAR_DATA);

//	while(*String_Index != 1021)
//	{
//		String_Buffer[(*String_Index)++] = 'A';
//	}
//	String_Buffer[(*String_Index)++] = ',';
	String_Buffer[(*String_Index)++] = '\r';
	String_Buffer[(*String_Index)++] = '\n';

//	uint8_t Buffer[20];
//	uint16_t Len = sprintf(Buffer,"%d\r\n",*String_Index);
//	BMS_Debug_COM_Write_Data(Buffer,Len);
	/* Write all the variables stored in the buffer to the SD card */

	if (f_write(&BMS_Log_File, String_Buffer, *String_Index, &BytesWritten) != FR_OK)
	{
		Result = RESULT_ERROR;
	}

	if(f_sync(&BMS_Log_File) != FR_OK)
	{
		Result = RESULT_ERROR;
	}
	return Result;
}

/**
 * @brief  Function to write the updated counts i.e. Current_file_count and Power_Up_Number
 * 		   current directory and file counts along with header data.
 * @param  Offset			: The cursor position at which data is to be written to the file
 * @param  Decimal_Number	: The decimal number which is to be stored in the file
 * @retval RESULT_OK	: Writing counts to the log summary file is successful
 *		   RESULT_ERROR	: Writing counts to the log summary file is unsuccessful
 */
uint8_t Write_Count_Log_Summary_File(uint32_t Offset,uint16_t Decimal_Number)
{
	char Buffer[6];

	/* Clear the buffer so as to avoid garbage write to the file (we may use sizeof operator)*/
	for(int temp = 0; temp< MAX_DIGITS_IN_COUNT; temp++)
		Buffer[temp] = '\0';

	/* Go to the known offset to update the count value*/
	f_lseek(&Summary_File,Offset);
	/* Convert decimal number to string */
	itoa(Decimal_Number,(char*)Buffer,DECIMAL_BASE);

	/* write the updated value to file at respective cursor position */
	if(f_write(&Summary_File,Buffer,MAX_DIGITS_IN_COUNT,&BytesWritten) != FR_OK)
	{
		return RESULT_ERROR;
	}

	/* make sure data is written to the file by calling f_sync function */
	f_sync(&Summary_File);

	return RESULT_OK;
}

/**
 * @brief  Function to write the updated counts i.e. Current_file_count and Power_Up_Number
 * @param  None
 * @retval None
 */
void Stop_Log()
{
	uint8_t length;
	memset(String_Buffer,0,sizeof(String_Buffer));
	f_lseek(&BMS_Log_File,Stop_Time_Cursor);

	if(1)
		length = sprintf(GPS_Date_Time, "%02d-%02d-%04d %02d:%02d:%02d",26,10,2017,19,45,05);
	else
		length = sprintf(GPS_Date_Time, "%02d-%02d-%04d %02d:%02d:%02d", 0,0,0,0,0,0);

	f_write(&BMS_Log_File, GPS_Date_Time, (UINT)length, &BytesWritten);
	f_close(&BMS_Log_File);
	f_close(&Summary_File);
}

/**
 * @brief  Function to return the size of the file in bytes
 * @param  None
 * @retval Size of the file in bytes
 */
unsigned long Get_BMS_Log_File_Size()
{
	return f_size(&BMS_Log_File);
}

/**
 * @brief  Function to convert the user variables into CSV format
 * @param  data_array	: pointer to the array of integer/float/long/short float/short int
 * @param  dst_array 	: pointer to the array of character to which resulting string is stored
 * @param  count 	  	: number of data values to be converted into string
 * @param  offset    	: location from which converted characters are stored
 * @param  data_type 	: it can be INT_DATA/SHORT_INT_DATA/FLOAT_DATA/SHORT_FLOAT_DATA/LONG/CHAR_DATA
 * @retval None
 */
void log_sprintf(void *data_array,char *dst_array,uint8_t *count,uint32_t *offset,uint8_t data_type)
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
#if SF_DECIMAL_POINT_PLACE == 2
					data *= 100;
#else
					data *= 1000;
#endif
					lcl_int = (int)data;

				while(lcl_int != 0 && lcl_index < data_type - 1)
				{
					if(lcl_index == SF_DECIMAL_POINT_PLACE)
						dst_array[*offset + data_type - ++lcl_index] = '.';
					else
					{
						dst_array[*offset + data_type - ++lcl_index] = (lcl_int % 10) + '0';
						lcl_int = lcl_int / 10;
					}

				}
				while(lcl_index != (data_type-1))
				{
					if(lcl_index == SF_DECIMAL_POINT_PLACE)
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

#if LF_DECIMAL_POINT_PLACE == 2
					data *= 100;
#else
					data *= 1000;
#endif
					lcl_int = (int)data;

				while(lcl_int != 0 && lcl_index < data_type - 1)
				{
					if(lcl_index == LF_DECIMAL_POINT_PLACE)
						dst_array[*offset + data_type - ++lcl_index] = '.';
					else
					{
						dst_array[*offset + data_type - ++lcl_index] = (lcl_int % 10) + '0';
						lcl_int = lcl_int / 10;
					}
				}
				while(lcl_index != (data_type-1))
				{
					if(lcl_index == LF_DECIMAL_POINT_PLACE)
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

