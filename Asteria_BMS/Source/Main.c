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

void Set_System_Clock_Frequency(void);

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

uint8_t Showtime[30] = "test_string\r";
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
//	GPIO_Init(GPIO_B,GPIO_PIN_4,GPIO_INPUT,NOPULL);

	BMS_Timers_Init();
	BMS_COM_Init();
	RTC_Init();

	RTC_Info.Day = WEDNESDAY;
	RTC_Info.Date = 0x05;
	RTC_Info.Month = APRIL;
	RTC_Info.Year = 0x17;

	RTC_Info.Hours = 0x13;
	RTC_Info.Minutes = 0x30;
	RTC_Info.Seconds = 0x00;

	RTC_Set_Date(&RTC_Info.Day,&RTC_Info.Date,&RTC_Info.Month,&RTC_Info.Year);
	RTC_Set_Time(&RTC_Info.Hours,&RTC_Info.Minutes,&RTC_Info.Seconds);

	while(1)
	{
		if((SysTickCounter % 1000) == 0 && Done == true)
		{
			BMS_COM_Write_Data("High\r", 5);
			Done = false;
//			if (GPIO_Read(GPIO_B, GPIO_PIN_4) == PIN_HIGH)
//			{
//				BMS_COM_Write_Data("High", 5);
//			}
//			else
//			{
//				BMS_COM_Write_Data("LOW", 5);
//			}
			//RTC_TimeShow(Showtime);
			//BMS_COM_Write_Data(Showtime,12);
//			Delay_Millis(5);
//			memset(Showtime,0,sizeof(Showtime));
		}

		BMS_COM_Read_Data(&RecData, 1);

		switch(RecData)
		{
			case 'A':
				if(Create_BMS_Log_File() == 1)
				{
					BMS_COM_Write_Data("Log_file_Created\r",17);
				}
				break;

			case 'B':
				Start_Log = true;
				BMS_COM_Write_Data("Log Started\r",13);
				break;

			case 'C':
				Start_Log = false;
				Stop_Log();
				BMS_COM_Write_Data("Log_Stopped\r",13);
				break;

			default:
				break;
		}

		RecData = 0;

		if(_1Hz_Flag == true)
		{
			if(Start_Log == true)
			{
				if (Log_All_Data() == 1)
				{
					GPIO_Write(GPIO_B, BOARD_LED, PIN_TOGGLE);
				}
			}
			_1Hz_Flag = false;
		}

		if (_50Hz_Flag == true )
		{
			_50Hz_Flag = false;
		}

//		Delay_Millis(50);
	}
}

void Set_System_Clock_Frequency(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

	  /* MSI is enabled after System reset, activate PLL with MSI as source */
	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
	  RCC_OscInitStruct.PLL.PLLM = 1;
	  RCC_OscInitStruct.PLL.PLLN = 40;
	  RCC_OscInitStruct.PLL.PLLR = 2;
	  RCC_OscInitStruct.PLL.PLLP = 7;
	  RCC_OscInitStruct.PLL.PLLQ = 4;
	  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	  {
	  }

	  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	     clocks dividers */
	  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	  {
	  }
}
