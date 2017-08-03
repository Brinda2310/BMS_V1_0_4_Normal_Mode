/*
 * BMS_Timing.c
 *
 *  Created on: 17-Jan-2017
 *      Author: NIKHIL
 */

#include <BMS_Timing.h>

/* Flags to monitor 25Hz and 1Hz loop */
bool _30Hz_Flag = false,_1Hz_Flag = false,_30_Hz_SMBUS_Flag = false;
static volatile int16_t Counter = 0;

/* Function to initialize the timer to 40mS. Timer value can be changed by changing the macro value
 * defined in BMS_Timing.h file */
void BMS_Timers_Init()
{
	/* Timer value set to 33ms i.e. interrupt will occur at every 33ms and makes the flag true in ISR */
	Timer_Init(TIMER_2,_33ms_PERIOD);
}

/* Function to return the time from boot in milliseconds */
double Get_System_Time_Seconds()
{
	return (SysTickCounter/1000);
}

uint64_t Get_System_Time_Millis()
{
	return SysTickCounter;
}
/* Function callback for 30Hz timer interrupt */
void TIM2_PeriodElapsedCallback()
{
//	_30Hz_Flag = true;
	_30_Hz_SMBUS_Flag = true;

	Counter++;

	if (Counter >= _1_SECONDS)
	{
		_1Hz_Flag = true;
		Counter = 0;
	}
}
