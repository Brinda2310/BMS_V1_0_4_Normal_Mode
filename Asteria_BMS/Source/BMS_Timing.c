/*
 * BMS_Timing.c
 *
 *  Created on: 17-Jan-2017
 *      Author: NIKHIL
 */

#include <BMS_Timing.h>
#include <string.h>

bool flag = false;
uint8_t count = 0;
/* after 20 second force BMS IC to Doze mode. if BMS in Doze mode, BMS_Mode_Status flag true */
bool BMS_Mode_status = false;

/* Flags to monitor 25Hz and 1Hz loop; 25Hz flag will be true after every 40ms time period;
 * 1Hz flag will be true after every one second */
bool _25Hz_Flag = false,_1Hz_Flag = false;

/* Variable to count to number of 40ms duration to achieve other tumer delays */
static volatile int16_t Counter = 0;

uint8_t BMS_Idle_Time_Count = 0;
uint8_t BMS_Doze_Time_Count = 0;

/* variables to log the loop rate on SD card; Loop_Rate_Counter will be incremented after every 40ms duration
 * in main.c file */
uint8_t Loop_Rate_Counter = 0, Loop_Rate_Log_Counter = 0;

/**
 * @brief  Function to initialize the timer to 40mS. Timer value can be changed by changing the macro value
 * 		   defined in BMS_Timing.h file
 * @param  None
 * @retval None
 */
void BMS_Timers_Init()
{
	/* Timer value set to 40ms i.e. interrupt will occur at every 40ms and makes the flag true in ISR */
	Timer_Init(TIMER_2,_40ms_PERIOD_20MHz_SYS_Clock);

	/* Timer value set to 1 second */
	Timer_Init(TIMER_6,_1sec_PERIOD_20MHz_SYS_Clock);
}

/**
 * @brief  Function to return the time from boot in seconds
 * @param  None
 * @retval The timer value in seconds
 */
double Get_System_Time_Seconds()
{
	return (SysTickCounter/1000);
}

/**
 * @brief  Function to return the time from boot in milliseconds
 * @param  None
 * @retval The timer value in milliseconds
 */
uint64_t Get_System_Time_Millis()
{
	return SysTickCounter;
}

/**
 * @brief  Function callback for 25Hz timer interrupt
 * @param  None
 * @retval None
 */
void TIM2_PeriodElapsedCallback()
{
	/* This variable is used in the main loop for 25Hz tasks */
	_25Hz_Flag = true;

	Counter++;

	/* Count the 40ms durations to create one second delay and the same flag is used in main loop for 1Hz tasks */
	if (Counter >= _1_SECONDS)
	{
		_1Hz_Flag = true;
		Counter = 0;
	}
}

/**
 * @brief  Function callback for 1Hz timer interrupt
 * @param  None
 * @retval None
 */
void TIM6_PeriodElapsedCallback(void)
{
	flag = true;

//	BMS_Idle_Time_Count++;
//	BMS_Doze_Time_Count++;

	/* after 20 second force to BMS IC to Idle mode */
//	if(BMS_Doze_Time_Count >= 20)
//	{
//		count++;
//
//		if(count == 1)
//		{
//			BMS_Mode_status = true;
//		}
//	}
}
