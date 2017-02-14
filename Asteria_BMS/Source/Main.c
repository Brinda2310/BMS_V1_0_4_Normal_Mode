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

#define THOUSANDS_DIGIT_POS					6
#define HUNDREDS_DIGIT_POS					(THOUSANDS_DIGIT_POS + 1)
#define TENS_DIGIT_POS						(HUNDREDS_DIGIT_POS + 1)
#define UNITS_DIGIT_POS						(TENS_DIGIT_POS + 1)

#ifdef BMS_VERSION
	#define FLASH_USER_START_ADDR   		ADDR_FLASH_PAGE_16
	#define FLASH_USER_END_ADDR     		(ADDR_FLASH_PAGE_16+16)
#endif

void Set_System_Clock_Frequency(void);

uint16_t Counter = 0;
uint8_t Units_Digit = 0,Tens_Digit= 0, Hundreds_Digit = 0,Thousands_Digit = 0;

FATFS FatFs;
FIL File;
FRESULT Result;
UINT BytesWritten;

char filename[50];

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

uint8_t Buffer[50] = "New Data to previous file\r\n";
uint8_t Folder_Name[3] = {'_','1'};
uint8_t Showtime[30];
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

	if(f_mount(&FatFs,"0",1) == FR_OK)
	{
		HAL_GPIO_TogglePin(GPIOB,BOARD_LED);
	}

	if(f_mkdir("_1") == FR_OK)
	{
		BMS_COM_Write_Data("Directory", 9);
	}

	filename[Counter++] = '0';
	filename[Counter++] = ':';
	filename[Counter++] = '/';
	filename[Counter++] = Folder_Name[0];
	filename[Counter++] = Folder_Name[1];
	filename[Counter++] = '/';

	filename[THOUSANDS_DIGIT_POS] = '0';
	filename[HUNDREDS_DIGIT_POS] = '0';
	filename[TENS_DIGIT_POS] = '0';
	filename[UNITS_DIGIT_POS] = '0';

	Counter += 4;

	filename[Counter++] = '.';
	filename[Counter++] = 't';
	filename[Counter++] = 'x';
	filename[Counter++] = 't';
	filename[Counter++] = '\0';

	if(f_open(&File,filename,FA_OPEN_ALWAYS | FA_WRITE) == FR_OK)
	{
		HAL_GPIO_TogglePin(GPIOB,BOARD_LED);
	}

//	if(f_lseek(&File,File.fsize) == FR_OK)
//	{
//		BMS_COM_Write_Data("File seek done\r",15);
//		HAL_GPIO_TogglePin(GPIOB,BOARD_LED);
//	}

	while(1)
	{
		if((SysTickCounter % 1000) == 0)
		{
			RTC_TimeShow(Showtime);
			BMS_COM_Write_Data(Showtime,8);
		}

		if(_1Hz_Flag == true)
		{
			Counter++;
			_1Hz_Flag = false;
		}

		if(Counter >= 1)
		{
			filename[UNITS_DIGIT_POS] = ++Units_Digit + '0';

			if (Units_Digit > 9)
			{
				Units_Digit = 0;
				filename[UNITS_DIGIT_POS] = '0';
				filename[TENS_DIGIT_POS] = ++Tens_Digit+ '0';
			}

			if(Tens_Digit > 9)
			{
				Units_Digit = 0;
				Tens_Digit = 0;
				filename[UNITS_DIGIT_POS] = '0';
				filename[TENS_DIGIT_POS] = '0';
				filename[HUNDREDS_DIGIT_POS] = ++Hundreds_Digit+ '0';
			}
			if(Hundreds_Digit > 9)
			{
				Units_Digit = 0;
				Tens_Digit = 0;
				Hundreds_Digit = 0;
				filename[THOUSANDS_DIGIT_POS] = ++Thousands_Digit + '0';
			}

			if (f_open(&File, filename, FA_CREATE_ALWAYS | FA_WRITE | FA_READ)== FR_OK)
			{
				BMS_COM_Write_Data(filename, 15);
			}
			Counter = 0;
		}

		if (_50Hz_Flag == true )
		{
			if (f_write(&File, Buffer, strlen((char*)Buffer), &BytesWritten)== FR_OK)
			{
				HAL_GPIO_TogglePin(GPIOB, BOARD_LED);
				//GPIO_Write(GPIO_B,BOARD_LED,PIN_TOGGLE);
				f_sync(&File);
			}
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
