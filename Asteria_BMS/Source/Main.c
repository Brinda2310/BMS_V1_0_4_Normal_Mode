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

uint8_t data[4] = {0x11,0x22,0x33,0x44};
uint8_t ReadEEPROMData[10];
uint8_t Counter = 0;

const uint8_t BMS_Firmware_Version[3] =
{
		0,			//Major release version--modified when code is being merged to Master branch.
		0,			//Current stable code release-- modified when code is being merged to Develop branch.
		1			//Beta code release--modified when code is being merged to test_develop branch.
};

bool Start_Log = false;
uint8_t Status_Byte = 0x00;

int main(void)
{
	HAL_Init();
	Set_System_Clock_Frequency();

	GPIO_Init(GPIO_B,BOARD_LED,GPIO_OUTPUT,PULLUP);

	Delay_Millis(5000);
	BMS_Timers_Init();
	BMS_COM_Init();
	ISL94203_Init();
	RTC_Init();

//	RTC_Info.Day = THURSDAY;
//	RTC_Info.Date = 0x20;
//	RTC_Info.Month = APRIL;
//	RTC_Info.Year = 0x17;
//
//	RTC_Info.Hours = 0x18;
//	RTC_Info.Minutes = 0x12;
//	RTC_Info.Seconds = 0x40;
//
//	RTC_Set_Date(&RTC_Info.Day,&RTC_Info.Date,&RTC_Info.Month,&RTC_Info.Year);
//	RTC_Set_Time(&RTC_Info.Hours,&RTC_Info.Minutes,&RTC_Info.Seconds);
//
	ISL94203_User_EEPROM_Write(USER_EEPROM_START_ADDRESS,data,4);

	memset(data,0,sizeof(data));

	if(Create_BMS_Log_File() == 1)
	{
		BMS_COM_Write_Data("Log_file_Created\r", 17);
		Start_Log = true;
	}
	else
	{
		BMS_COM_Write_Data("SD Card Not Present\r", 20);
	}

	while(1)
	{
		BMS_COM_Read_Data(&RecData, 1);

		switch(RecData)
		{
			case 'A':
				Start_Log = false;
				f_close(&BMS_Log_File);
				break;
			case 'B':
				f_mount(&FatFs,"0",1);
				f_open(&BMS_Log_File,File_Name,FA_OPEN_EXISTING | FA_WRITE | FA_READ);
				f_lseek(&BMS_Log_File,BMS_Log_File.fsize);
				Start_Log = true;
				break;
			case 'C':
				Sleep_Mode = true;
				Enter_Sleep_Mode();
				break;
			case 'D':
				ISL94203_User_EEPROM_Read(USER_EEPROM_START_ADDRESS,ReadEEPROMData, 4);
				BMS_COM_Write_Data(ReadEEPROMData, 4);
				Delay_Millis(5);
				memset(ReadEEPROMData, 0, sizeof(ReadEEPROMData));
				break;
			case 'E':
				ISL94203_Force_Sleep();
				break;

//			default:
//				BMS_COM_Write_Data("Wrong Input\r",12);
//				break;
		}

		RecData = 0;

		if (_50Hz_Flag == true)
		{
			Counter++;
			_50Hz_Flag = false;
		}

		if(Counter >= 50)
		{
			if ((Log_All_Data() == 1)  && Start_Log == 1)
			{
				GPIO_Write(GPIO_B, BOARD_LED, PIN_TOGGLE);
			}

			ISL94203_RAM_Status_Register(RAM_0x83_STATUS,&Status_Byte);
			if((Status_Byte & IS_ISL_IN_SLEEP) == IS_ISL_IN_SLEEP)
			{
				BMS_COM_Write_Data("Sleep\r", 6);
			}
			else
				BMS_COM_Write_Data("No Sleep\r", 9);
			Counter = 0;
			Status_Byte = 0;
			RTC_TimeShow(Showtime);
			Delay_Millis(2);
			BMS_COM_Write_Data(Showtime, strlen((char*) Showtime));
			Delay_Millis(2);
			memset(Showtime, 0, sizeof(Showtime));
		}
	}
}

