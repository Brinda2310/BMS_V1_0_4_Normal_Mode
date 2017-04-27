/**
 ******************************************************************************
 * @file    Main.c
 * @author  Nikhil Ingale
 * @version V1.0.0
 * @date    01-December-2013
 * @brief   Default main function.
 ******************************************************************************
*/
#include <BMS_ASIC.h>
#include <BMS_Data_Log.h>
#include <BMS_Serial_Communication.h>
#include <BMS_GPIOs.h>
#include <BMS_Timing.h>
#include "FLASH_API.h"
#include "RTC_API.h"
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

const uint8_t BMS_Firmware_Version[3] =
{
		0,			//Major release version--modified when code is being merged to Master branch.
		0,			//Current stable code release-- modified when code is being merged to Develop branch.
		1			//Beta code release--modified when code is being merged to test_develop branch.
};

bool Start_Log = false;
RTC_Data RTC_Info;

uint8_t RecData = 0;

uint8_t data[4] = {0x11,0x22,0x33,0x44};
uint8_t ReadEEPROMData[10];
uint16_t Time_Count_Switch_Press = 0;
uint16_t Time_Count_BMS_Sleep = 0;
bool ISL_Sleep = false;
bool Short_Time_Elapsed = false,Long_Time_Elapsed = false;

char Buffer[20];

int main(void)
{
	HAL_Init();
	Set_System_Clock_Frequency();

	Delay_Millis(5000);
	BMS_Timers_Init();
#if DEBUG_COM == ENABLE
	BMS_Debug_COM_Init();
#endif
	BMS_Status_LEDs_Init();
	BMS_Switch_Init();

	BMS_ASIC_Init();
	RTC_Init();

	RTC_Info.Day = FRIDAY;
	RTC_Info.Date = 0x21;
	RTC_Info.Month = APRIL;
	RTC_Info.Year = 0x17;

	RTC_Info.Hours = 0x12;
	RTC_Info.Minutes = 0x58;
	RTC_Info.Seconds = 0x15;

	RTC_Set_Date(&RTC_Info.Day,&RTC_Info.Date,&RTC_Info.Month,&RTC_Info.Year);
	RTC_Set_Time(&RTC_Info.Hours,&RTC_Info.Minutes,&RTC_Info.Seconds);

	BMS_User_EEPROM_Write(USER_EEPROM_START_ADDRESS,data,4);

	memset(data,0,sizeof(data));

	if(Create_BMS_Log_File() == 1)
	{
		BMS_Debug_COM_Write_Data("Log_file_Created\r", 17);
		Start_Log = true;
	}
	else
	{
		BMS_Debug_COM_Write_Data("SD Card Not Present\r", 20);
	}

//	BMS_Estimate_Initial_Capacity();

	while(1)
	{
		BMS_Debug_COM_Read_Data(&RecData, 1);

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
				BMS_User_EEPROM_Read(USER_EEPROM_START_ADDRESS,ReadEEPROMData, 4);
				BMS_Debug_COM_Write_Data(ReadEEPROMData, 4);
				Delay_Millis(5);
				memset(ReadEEPROMData, 0, sizeof(ReadEEPROMData));
				break;
			case 'E':
				BMS_Force_Sleep();
				break;
			case 'F':
				break;

//			default:
//				BMS_Debug_COM_Write_Data("Wrong Input\r",12);
//				break;
		}

		RecData = 0;

		if (_25Hz_Flag == true)
		{
			if (BMS_Read_Switch_Status() == PRESSED)
			{
				Time_Count_Switch_Press++;
//				sprintf(Buffer,"%d\r",Time_Count_Switch_Press);
//				BMS_Debug_COM_Write_Data(Buffer,strlen(Buffer));
//				Delay_Millis(2);
				if(Time_Count_Switch_Press >= LONG_PEROID)
				{
					Long_Time_Elapsed = true;
				}

				if(Time_Count_Switch_Press >= SHORT_PERIOD)
				{
					Short_Time_Elapsed = true;
				}
			}
			else
			{
				if(Long_Time_Elapsed == true)
				{
					BMS_Show_LED_Status(2);
					Short_Time_Elapsed = false;
					Long_Time_Elapsed = false;
				}
				if(Short_Time_Elapsed == true)
				{
					Short_Time_Elapsed = false;
					BMS_Show_LED_Status(1);
				}
				Time_Count_Switch_Press = 0;
			}

			BMS_Read_Cell_Voltages();
			BMS_Read_Pack_Voltage();
			BMS_Read_Pack_Current();
			BMS_Read_Pack_Temperature();
			BMS_Read_RAM_Status_Register();

			if(((uint16_t)Get_BMS_Pack_Current() < 50) && ((uint16_t)Get_BMS_Pack_Voltage()) < 25 && ISL_Sleep == false)
			{
				Time_Count_BMS_Sleep++;
				sprintf(Buffer, "%d\r", Time_Count_BMS_Sleep);
				BMS_Debug_COM_Write_Data(Buffer, strlen(Buffer));
				Delay_Millis(5);
//
				if(Time_Count_BMS_Sleep >= LOW_CONSUMPTION_DELAY)
				{
					ISL_Sleep = true;
					BMS_Debug_COM_Write_Data("Went to sleep\r",14);
//					BMS_Force_Sleep();
				}
			}
			else if (((uint16_t)Get_BMS_Pack_Current() > 50))
			{
				Time_Count_BMS_Sleep = 0;
				ISL_Sleep = false;
			}

			_25Hz_Flag = false;
		}


		if(_1Hz_Flag == true && ISL_Sleep == false)
		{
			char Buffer[200];
			Delay_Millis(2);
			uint8_t Length1 = sprintf(Buffer,"Pack_Voltage = %0.3fV\r",Get_BMS_Pack_Voltage());
			BMS_Debug_COM_Write_Data(Buffer,Length1);
			Delay_Millis(5);

			Length1 = sprintf(Buffer,"Pack_Current = %0.3fmA\r",Get_BMS_Pack_Current());
			BMS_Debug_COM_Write_Data(Buffer,Length1);
			Delay_Millis(5);

			Length1 = sprintf(Buffer,"Pack_Temp = %0.3fmV\r",Get_BMS_Pack_Temperature());
			BMS_Debug_COM_Write_Data(Buffer,Length1);
			Delay_Millis(2);

			uint16_t Length = 0;
			Length += sprintf(Buffer, "Cell1_V = %0.3fV\r", Get_Cell1_Voltage());
			Length += sprintf(&Buffer[Length], "Cell2_V = %0.3fV\r",Get_Cell2_Voltage());
			Length += sprintf(&Buffer[Length], "Cell3_V = %0.3fV\r",Get_Cell3_Voltage());
			Length += sprintf(&Buffer[Length], "Cell6_V = %0.3fV\r",Get_Cell6_Voltage());
			Length += sprintf(&Buffer[Length], "Cell7_V = %0.3fV\r",Get_Cell7_Voltage());
			Length += sprintf(&Buffer[Length], "Cell8_V = %0.3fV\r",Get_Cell8_Voltage());

			BMS_Debug_COM_Write_Data(Buffer, Length);
			Delay_Millis(10);

			if ((Log_All_Data() == 1)  && Start_Log == 1)
			{
				BMS_Debug_COM_Write_Data("Written\r",8);
			}
			else
			{
				BMS_Debug_COM_Write_Data("Write Error\r",12);
			}
//			Delay_Millis(2);
//			if(Status_Flag.BMS_In_Sleep == YES)
//			{
//				BMS_Debug_COM_Write_Data("In sleep mode\r",14);
//			}
//			else
//			{
//				BMS_Debug_COM_Write_Data("Non-sleep\r",10);
//			}
//			Delay_Millis(2);
//			if(Status_Flag.Pack_Discharging == YES)
//			{
//				BMS_Debug_COM_Write_Data("Discharging\r",14);
//			}
//			else
//				BMS_Debug_COM_Write_Data("Low power mode\r",15);

			_1Hz_Flag = false;
			memset(Buffer, 0, sizeof(Buffer));
		}
	}
}

