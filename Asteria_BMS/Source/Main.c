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

#define THOUSANDS_DIGIT_POS					6
#define HUNDREDS_DIGIT_POS					(THOUSANDS_DIGIT_POS + 1)
#define TENS_DIGIT_POS						(HUNDREDS_DIGIT_POS + 1)
#define UNITS_DIGIT_POS						(TENS_DIGIT_POS + 1)

#ifdef BMS_VERSION
	#define FLASH_USER_START_ADDR   		ADDR_FLASH_PAGE_16
	#define FLASH_USER_END_ADDR     		(ADDR_FLASH_PAGE_16+16)
#endif

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

int main(void)
{
	int8_t Counter = 0;

	HAL_Init();
	Set_System_Clock_Frequency();
	GPIO_Init(GPIO_B,BOARD_LED,GPIO_OUTPUT,PULLUP);
	BMS_Timers_Init();
	BMS_COM_Init();
	RTC_Init();

	RTC_Info.Day = TUESDAY;
	RTC_Info.Date = 0x14;
	RTC_Info.Month = FEBRUARY;
	RTC_Info.Year = 0x17;

	RTC_Info.Hours = 0x11;
	RTC_Info.Minutes = 0x28;
	RTC_Info.Seconds = 0x00;

	RTC_Set_Date(&RTC_Info.Day,&RTC_Info.Date,&RTC_Info.Month,&RTC_Info.Year);
	RTC_Set_Time(&RTC_Info.Hours,&RTC_Info.Minutes,&RTC_Info.Seconds);

	Create_Log_Summary_File();
	//Create_Log_File();
	Update_Log_Summary_File();

//	if(f_lseek(&File,File.fsize) == FR_OK)
//	{
//		BMS_COM_Write_Data("File seek done\r",15);
//		HAL_GPIO_TogglePin(GPIOB,BOARD_LED);
//	}

	while(1)
	{
		if((SysTickCounter % 1000) == 0)
		{
			//RTC_TimeShow(Showtime);
			BMS_COM_Write_Data(Showtime,12);
//			Delay_Millis(5);
//			memset(Showtime,0,sizeof(Showtime));
		}

		if(_1Hz_Flag == true)
		{
			_1Hz_Flag = false;
		}

		if (_50Hz_Flag == true )
		{

			_50Hz_Flag = false;
		}

/* Section of  code to delete the files on disk */
//		BMS_COM_Read_Data(&RecData,1);
//		if(RecData == 'A')
//		{
//			if(f_unlink("0:/Test_Delete.txt") == FR_OK)
//			{
//				//HAL_GPIO_TogglePin(GPIOB,BOARD_LED);
//				BMS_COM_Write_Data("Error\n\r",7);
//			}
//			else
//			{
//				BMS_COM_Write_Data("Success\n\r",9);
//				//HAL_GPIO_WritePin(GPIOB,BOARD_LED,PIN_HIGH);
//			}
//
//		}
//		else if (RecData == 'B')
//		{
//			BMS_COM_Write_Data("Padded string\n\r",15);
//		}
//		RecData = 0;
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
