/*
 * Data_Log.c
 *
 *  Created on: 16-Feb-2017
 *      Author: NIKHIL
 */

#include "Data_Log.h"

FATFS FatFs;
FIL File;
FRESULT Result;
UINT BytesWritten;

uint8_t Units_Digit = 0,Tens_Digit= 0, Hundreds_Digit = 0,Thousands_Digit = 0;
char Log_File_Name[15] = "0:/_";
uint32_t String_Index = 0;
uint32_t File_Number = 0;

uint8_t File_Data[100];

uint16_t Counter = 0;

uint8_t Create_Log_File(uint8_t *File_Name)
{
	int8_t Number_in_String[8],String_Length = 0;
	char  String_Buffer[100];

	if (f_mount(&FatFs, "0", 1) == FR_OK)
		return RESULT_ERROR;

	itoa(File_Number,(char*)Number_in_String,10);
    String_Length = strlen((char*)Number_in_String);

	Counter = 4;
	for(int index = 0 ; index < String_Length ; index++)
	{
		Log_File_Name[Counter++] = Number_in_String[index];
	}
	Log_File_Name[Counter++] = '.';
	Log_File_Name[Counter++] = 't';
	Log_File_Name[Counter++] = 'x';
	Log_File_Name[Counter++] = 't';
	Log_File_Name[Counter++] = '\0';

	if (f_open(&File, Log_File_Name, FA_OPEN_ALWAYS | FA_WRITE) != FR_OK)
		return RESULT_ERROR;

	String_Index += sprintf(String_Buffer,"GPS_Date,Start_Time,End_Time,Initial_Cell_Voltage,Final_Cell_Voltage,Initial_Pack_Voltage,");
//	f_write(&File_Name,String_Buffer,String_Index,&BytesWritten);

	String_Index += sprintf(&String_Buffer[String_Index],"Final_Pack_Voltage,Total_Capacity,Initial_Capacity,Final_Capacity,Battery C/D Date,");
//	f_write(&File_Name,String_Buffer,String_Index,&BytesWritten);

	String_Index += sprintf(&String_Buffer[String_Index],"Packed_cycles_used,Charging/Discharging,Min_Temperature,Max_Temperature,");
//	f_write(&File_Name,String_Buffer,String_Index,&BytesWritten);

	String_Index += sprintf(&String_Buffer[String_Index],"Final_Pack_Voltage,Flight_Time,Health_Status_Register\r\n");

	f_write(&File,String_Buffer,String_Index,&BytesWritten);

	return RESULT_OK;
}

//void Write_Data_To_File(uint8_t *Data)
//{
//	if (f_write(&File, Buffer, strlen((char*) Buffer), &BytesWritten)== FR_OK)
//	{
//		HAL_GPIO_TogglePin(GPIOB, BOARD_LED);
//		f_sync(&File);
//	}
//}
//static void Maintain_Directory()
//{
//	if(Counter >= 1)
//	{
//		filename[UNITS_DIGIT_POS] = ++Units_Digit + '0';
//
//		if (Units_Digit > 9)
//		{
//			Units_Digit = 0;
//			filename[UNITS_DIGIT_POS] = '0';
//			filename[TENS_DIGIT_POS] = ++Tens_Digit+ '0';
//		}
//
//		if(Tens_Digit > 9)
//		{
//			Units_Digit = 0;
//			Tens_Digit = 0;
//			filename[UNITS_DIGIT_POS] = '0';
//			filename[TENS_DIGIT_POS] = '0';
//			filename[HUNDREDS_DIGIT_POS] = ++Hundreds_Digit+ '0';
//		}
//		if(Hundreds_Digit > 9)
//		{
//			Units_Digit = 0;
//			Tens_Digit = 0;
//			Hundreds_Digit = 0;
//			filename[THOUSANDS_DIGIT_POS] = ++Thousands_Digit + '0';
//		}
//
//		if (f_open(&File, filename, FA_CREATE_ALWAYS | FA_WRITE | FA_READ)== FR_OK)
//		{
//			BMS_COM_Write_Data(filename, 15);
//		}
//		Counter = 0;
//	}
//}

uint8_t Create_Log_Summary_File()
{
	uint32_t Rx_Flash_Data = 0;
	uint64_t Tx_Flash_Data = 0;
	uint8_t Summary_File_Data[100];

	MCU_Flash_Read(DIRECTORY_NAME_START_ADDRESS,DIRECTORY_NAME_END_ADDRESS,&Rx_Flash_Data);

	if(Rx_Flash_Data != 0x00000001)
	{
		Tx_Flash_Data = 0x000000001;
		MCU_Flash_Write(DIRECTORY_NAME_START_ADDRESS,DIRECTORY_NAME_END_ADDRESS,&Tx_Flash_Data);

		if(f_mount(&FatFs,"0",1) != FR_OK)
			return RESULT_ERROR;

		if(f_open(&File,"0:/Log_Summary_File.txt",FA_OPEN_ALWAYS | FA_WRITE | FA_READ) != FR_OK)
			return RESULT_ERROR;

		if(f_write(&File,Summary_File_Data,sizeof(Summary_File_Data),&BytesWritten) != FR_OK)
			return RESULT_ERROR;
	}

return RESULT_OK;
}

uint8_t Update_Log_Summary_File()
{
	uint8_t Rx_Data[20];
	int8_t Timeout = 30,i = 0,lcl_index = 0;
	uint8_t File_Name[] = "File_Count:";
	bool loop_break = false;

	if(f_mount(&FatFs,"0",1) != FR_OK)
		return RESULT_ERROR;

	if(f_open(&File,"0:/Log_Summary_File.txt",FA_OPEN_EXISTING | FA_WRITE | FA_READ) != FR_OK)
		return RESULT_ERROR;

	while((Rx_Data[lcl_index] != 0x0D) && (Timeout-- > 0))
	{
		if(f_read(&File,&Rx_Data[lcl_index],1,&BytesWritten) != FR_OK)
		{
			if(Rx_Data[lcl_index] != ':' && loop_break == false)
			{
				if (Rx_Data[lcl_index] != File_Name[lcl_index])
					return RESULT_ERROR;
			}
			else
			{
				loop_break = true;
				if(Rx_Data[lcl_index] != ':')
					File_Name[i++] = Rx_Data[lcl_index];
			}
		}
		lcl_index++;
	}

	if(Timeout <= 0)
		return RESULT_ERROR;

	File_Number = atoi((char*)File_Name);

	File_Number++;

	return RESULT_OK;
}


