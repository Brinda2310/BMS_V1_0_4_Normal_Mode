/*
 * Data_Log.c
 *
 *  Created on: 16-Feb-2017
 *      Author: NIKHIL
 */

#include "Data_Log.h"

DIR Directory;
FATFS FatFs;
FIL File;
FRESULT Result;
UINT BytesWritten;

Log_Vars Log_Variables;

static uint8_t Directory_Name[10] = "_1";
uint8_t Delete_Dir_Count = 0;
uint8_t Units_Digit = 0,Tens_Digit= 0, Hundreds_Digit = 0,Thousands_Digit = 0;
char Log_File_Name[30];
uint32_t String_Index = 0;
uint32_t File_Number = 0,Directory_Number = 1;
char  String_Buffer[512];
//uint8_t File_Data[100];

uint8_t Create_Log_File()
{
	int8_t Number_in_String[8],String_Length = 0;
	uint8_t Counter = 0;
	uint8_t Dir_Length = 0;

	Get_File_Dir_Counts();

	if(f_opendir(&Directory,(char*)Directory_Name) != FR_OK)
		f_mkdir((char*)Directory_Name);

	if(File_Number >= 5)
	{
		File_Number = 0;
		f_closedir(&Directory);

		Directory_Number = atoi((char*)(Directory_Name + 1));
		Directory_Number++;

		itoa(Directory_Number,(char*)(Directory_Name+1),10);
		f_mkdir((char*)Directory_Name);
	}

	itoa(Directory_Number,&Directory_Name[1],10);

	Dir_Length = strlen((char*)Directory_Name);

	Log_File_Name[Counter++] = '0';
	Log_File_Name[Counter++] = ':';
	Log_File_Name[Counter++] = '/';

	for(int index = 0 ; index < Dir_Length; index++)
	{
		Log_File_Name[Counter++] = Directory_Name[index];
	}

	Log_File_Name[Counter++] = '/';
	Log_File_Name[Counter++] = '_';

	File_Number++;
	itoa(File_Number,(char*)Number_in_String,10);
    String_Length = strlen((char*)Number_in_String);

	for(int index = 0 ; index < String_Length ; index++)
	{
		Log_File_Name[Counter++] = Number_in_String[index];
	}
	Log_File_Name[Counter++] = '.';
	Log_File_Name[Counter++] = 't';
	Log_File_Name[Counter++] = 'x';
	Log_File_Name[Counter++] = 't';
	Log_File_Name[Counter++] = '\0';

	BMS_COM_Write_Data(Log_File_Name,strlen(Log_File_Name)+1);

	if (f_open(&File, Log_File_Name, FA_OPEN_ALWAYS | FA_WRITE) != FR_OK)
		return RESULT_ERROR;

	String_Index += sprintf(String_Buffer,"GPS_Date,Start_Time,End_Time,Initial_Cell_Voltage,Final_Cell_Voltage,Initial_Pack_Voltage,");
	String_Index += sprintf(&String_Buffer[String_Index],"Final_Pack_Voltage,Total_Capacity,Initial_Capacity,Final_Capacity,Battery C/D Date,");
	String_Index += sprintf(&String_Buffer[String_Index],"Pack_cycles_used,Charging/Discharging,Min_Temperature,Max_Temperature,");
	String_Index += sprintf(&String_Buffer[String_Index],"Flight_Time,Health_Status_Register\r\n");

	f_write(&File,String_Buffer,String_Index,&BytesWritten);
	f_sync(&File);

	Update_Log_Summary_File();

	memset(String_Buffer,0,sizeof(String_Buffer));
	String_Index = 0;

	return RESULT_OK;
}

void Write_Data_To_File()
{
	String_Index = sprintf(String_Buffer,"GPS_Date,Start_Time,End_Time,Initial_Cell_Voltage,Final_Cell_Voltage,Initial_Pack_Voltage\r\n");
	if (f_write(&File, String_Buffer, String_Index, &BytesWritten)== FR_OK)
	{
		f_sync(&File);
	}
	String_Index = 0;
}

uint8_t Delete_Directory()
{
	uint8_t Rx_Data[30],Data = 0;
	uint8_t Timeout = 30,lcl_index = 0, i =0;
	bool loop_break = false;
	uint8_t Directory_Name[30] = "Directory_Count:";

	if(f_mount(&FatFs,"0",1) != FR_OK)
		return RESULT_ERROR;

	if(f_open(&File,"0:/Log_Summary_File.txt",FA_OPEN_EXISTING | FA_WRITE | FA_READ) != FR_OK)
		return RESULT_ERROR;

	while (( Data != '\t') && (Timeout-- > 0))
	{
		if (f_read(&File, &Rx_Data[lcl_index], 1, &BytesWritten) == FR_OK)
		{
			Data = Rx_Data[lcl_index];
			if (Rx_Data[lcl_index] != ':' && loop_break == false)
			{
				if (Rx_Data[lcl_index] != Directory_Name[lcl_index])
					return RESULT_ERROR;
			}
			else
			{
				loop_break = true;
				if (Rx_Data[lcl_index] != ':')
					Directory_Name[i++] = Rx_Data[lcl_index];
			}
		}
		else
			return RESULT_ERROR;
		lcl_index++;
	}

	if (Timeout <= 0)
		return RESULT_ERROR;

	Directory_Number = atoi((char*)(Directory_Name));
	Directory_Number--;

	if(Delete_Dir_Count < Directory_Number)
		Delete_Dir_Count++;
	else
		return RESULT_ERROR;

	Directory_Name[0] = '_';
	itoa(Delete_Dir_Count,(char*)(Directory_Name+1),10);
	BMS_COM_Write_Data(Directory_Name,strlen((char*)Directory_Name));
	FRESULT res = f_unlink((char*)Directory_Name);

	Update_Log_Summary_File();

	return RESULT_OK;

}

uint8_t Create_Log_Summary_File()
{
	uint32_t Rx_Flash_Data = 0;
	uint64_t Tx_Flash_Data = 0;
	uint8_t Summary_File_Data[] = "File data to be filled\r\n";
	uint8_t Dir_String[] = "Directory_Count:1    \t";
	uint8_t File_String[] = "File_Count:0   \r\n";
	uint16_t String_Count = 0;

	MCU_Flash_Read(DIRECTORY_NAME_START_ADDRESS,DIRECTORY_NAME_END_ADDRESS,&Rx_Flash_Data);

	if(Rx_Flash_Data != 0x00000001)
	{
		Tx_Flash_Data = 0x000000001;
		MCU_Flash_Write(DIRECTORY_NAME_START_ADDRESS,DIRECTORY_NAME_END_ADDRESS,&Tx_Flash_Data);

		if(f_mount(&FatFs,"0",1) != FR_OK)
			return RESULT_ERROR;

		if(f_open(&File,"0:/Log_Summary_File.txt",FA_OPEN_ALWAYS | FA_WRITE | FA_READ) != FR_OK)
			return RESULT_ERROR;

		if((String_Count = f_write(&File,Dir_String,sizeof(Dir_String),&BytesWritten)) != FR_OK)
			return RESULT_ERROR;

		if(f_write(&File,File_String,(sizeof(File_String)-1),&BytesWritten) != FR_OK)
			return RESULT_ERROR;

		if(f_write(&File,Summary_File_Data,strlen((char*)Summary_File_Data),&BytesWritten) != FR_OK)
			return RESULT_ERROR;

		f_sync(&File);

//		BMS_COM_Write_Data("Log_Summary_File_OK\r",20);
	}

return RESULT_OK;
}

static uint8_t Update_Log_Summary_File()
{
	uint8_t Buffer[10];

	if(f_mount(&FatFs,"0",1) != FR_OK)
		return RESULT_ERROR;

	if(f_open(&File,"0:/Log_Summary_File.txt",FA_OPEN_EXISTING | FA_WRITE | FA_READ) != FR_OK)
		return RESULT_ERROR;

	itoa(File_Number,(char*)Buffer,10);
	f_lseek(&File,34);
	f_write(&File,Buffer,4,&BytesWritten);
	f_sync(&File);
	memset(Buffer,0,sizeof(Buffer));

	f_lseek(&File,16);
	itoa(Directory_Number,(char*)Buffer,10);
	f_write(&File,Buffer,4,&BytesWritten);
	f_sync(&File);


	return RESULT_OK;
}

uint8_t Get_File_Dir_Counts()
{
	uint8_t Rx_Data[10],Data = 0;
	char Count[10];
	uint8_t Timeout = 10,lcl_index = 0, i = 0;

	if(f_mount(&FatFs,"0",1) != FR_OK)
		return RESULT_ERROR;

	if(f_open(&File,"0:/Log_Summary_File.txt",FA_OPEN_EXISTING | FA_WRITE | FA_READ) != FR_OK)
		return RESULT_ERROR;

	f_lseek(&File,34);

	while (( Data != FILE_COUNT_TERMINATOR) && (Timeout-- > 0))
	{
		f_read(&File,&Rx_Data[lcl_index],1,&BytesWritten);
		Data = Rx_Data[lcl_index];
		Count[i++] = Data;
		lcl_index++;
	}

	if(Timeout < 0)
		return RESULT_ERROR;

	File_Number = atoi(Count);

	memset(Rx_Data,0,sizeof(Rx_Data));
	memset(Count,0,sizeof(Count));
	i = 0;

	f_lseek(&File,16);

	while ((Data != DIR_COUNT_TERMINATOR) && (Timeout-- > 0))
	{
		f_read(&File,&Rx_Data[lcl_index],1,&BytesWritten);
		Data = Rx_Data[lcl_index];
		Count[i++] = Data;
		lcl_index++;
	}

	if(Timeout < 0)
		return RESULT_ERROR;

	Directory_Number = atoi(Count);

return RESULT_OK;
}
