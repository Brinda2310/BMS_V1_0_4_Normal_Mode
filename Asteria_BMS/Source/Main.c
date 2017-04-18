/**
 ******************************************************************************
 * @file    Main.c
 * @author  Nikhil Ingale
 * @version V1.0.0
 * @date    01-December-2013
 * @brief   Default main function.
 ******************************************************************************
*/
#include "BMS_Timing.h"
#include "BMS_Communication.h"
#include "BMS_ISL94203.h"
#include "FLASH_API.h"
#include "RTC_API.h"
#include "GPIO_API.h"
#include "ff.h"
#include "Data_Log.h"
#include "Power_Management.h"

typedef struct
{
	uint8_t Day;
	uint8_t Date;
	uint8_t Month;
	uint8_t Year;
	uint8_t Hours;
	uint8_t Minutes;
	uint8_t Seconds;
}RTC_Data;

RTC_Data RTC_Info;

uint8_t Showtime[30];
uint8_t RecData = 0;

const uint8_t BMS_Firmware_Version[3] =
{
		0,			//Major release version--modified when code is being merged to Master branch.
		0,			//Current stable code release-- modified when code is being merged to Develop branch.
		1			//Beta code release--modified when code is being merged to test_develop branch.
};

bool Start_Log = false;

int main(void)
{
	HAL_Init();
	Set_System_Clock_Frequency();
	GPIO_Init(GPIO_B,BOARD_LED,GPIO_OUTPUT,PULLUP);

	BMS_Timers_Init();
	BMS_COM_Init();
	RTC_Init();

	RTC_Info.Day = WEDNESDAY;
	RTC_Info.Date = 0x14;
	RTC_Info.Month = APRIL;
	RTC_Info.Year = 0x17;

	RTC_Info.Hours = 0x11;
	RTC_Info.Minutes = 0x40;
	RTC_Info.Seconds = 0x00;

	RTC_Set_Date(&RTC_Info.Day,&RTC_Info.Date,&RTC_Info.Month,&RTC_Info.Year);
	RTC_Set_Time(&RTC_Info.Hours,&RTC_Info.Minutes,&RTC_Info.Seconds);

	if(Create_BMS_Log_File() == 1)
	{
		BMS_COM_Write_Data("Log_file_Created\r", 17);
	}
	else
		BMS_COM_Write_Data("Wrong\r", 6);
//	f_open(&BMS_Log_File,File_Name,FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	Start_Log = true;
	while(1)
	{
//		if(_50Hz_Flag == true)
//		{
//			RTC_TimeShow(Showtime);
//			BMS_COM_Write_Data(Showtime,9);
//			Delay_Millis(5);
//			memset(Showtime,0,sizeof(Showtime));
//		}

		BMS_COM_Read_Data(&RecData, 1);

		if(RecData == 'A')
		{
			f_close(&BMS_Log_File);
			Start_Log = false;
		}
		if(RecData == 'B')
		{
			f_mount(&FatFs,"0",1);
			f_open(&BMS_Log_File,File_Name,FA_OPEN_EXISTING | FA_WRITE | FA_READ);
			f_lseek(&BMS_Log_File,BMS_Log_File.fsize);
			Start_Log = true;
		}

		RecData = 0;

		if (_50Hz_Flag == true)
		{
			if (Log_All_Data() == 1 && Start_Log == true)
			{
				GPIO_Write(GPIO_B, BOARD_LED, PIN_TOGGLE);
			}
			_50Hz_Flag = false;
		}
	}
}

