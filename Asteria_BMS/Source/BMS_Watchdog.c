/*
 * BMS_Watchdog.c
 *
 *  Created on: 25-Oct-2017
 *      Author: NIKHIL
 */
#include <BMS_Watchdog.h>

/* Variable used to enable or disable the Watchdog functionality in the code and same is used for logging
 * watchdog enable/disable functionality */
bool BMS_Watchdog_Enable = false;

/**
 * @brief  Function to initialize Watchdog functionality
 * @param  None
 * @retval None
 */
void BMS_watchdog_Init(void)
{
	/* Initialize the watchdog timer with 2 seconds of timeout value */
	IWDG_Init(_2_SECONDS_WDG);

	/* As soon as this function is called this variable will become true so as to indicate the watchdog timer
	 * was enabled for this code execution (logged as variable in the SD card )*/
	BMS_Watchdog_Enable = true;
}

/**
 * @brief  Function to refresh the timer value; This resets the timer value back to 2 seconds and counts it down
 * @param  None
 * @retval None
 */
void BMS_Watchdog_Refresh(void)
{
	IWDG_Reset_Counter();
}
