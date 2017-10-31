/*
 * BMS_Watchdog.c
 *
 *  Created on: 25-Oct-2017
 *      Author: NIKHIL
 */
#include <BMS_Watchdog.h>

bool BMS_Watchdog_Enable = false;

void BMS_watchdog_Init(void)
{
	IWDG_Init(_2_SECONDS_WDG);
	BMS_Watchdog_Enable = true;
}

void BMS_Watchdog_Refresh(void)
{
	IWDG_Reset_Counter();
}
