/*
 * BMS_Timing.c
 *
 *  Created on: 17-Jan-2017
 *      Author: NIKHIL
 */

#include <BMS_Timing.h>

/* Flags to monitor 25Hz and 1Hz loop */
bool _25Hz_Flag = false,_1Hz_Flag = false;
static int16_t Counter = 0;

/* Function to initialize the timer to 40mS. Timer value can be changed by changing the macro value
 * defined in BMS_Timing.h file */
void BMS_Timers_Init()
{
	/* Timer value set to 40ms i.e. interrupt will occur at every 40ms and makes the flag true in ISR */
	Timer_Init(TIMER_2,_40ms_PERIOD);
}

/* Function to return the time from boot in milliseconds */
uint64_t Get_System_Time()
{
	return SysTickCounter;
}

/* Function callback for 25Hz timer interrupt */
void TIM2_PeriodElapsedCallback()
{
	_25Hz_Flag = true;
	Counter++;
	if (Counter >= 25)
	{
		_1Hz_Flag = true;
		Counter = 0;
	}
}
