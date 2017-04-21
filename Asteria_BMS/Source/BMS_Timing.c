/*
 * BMS_Timing.c
 *
 *  Created on: 17-Jan-2017
 *      Author: NIKHIL
 */

#include "BMS_Timing.h"
#include "stdbool.h"

bool _25Hz_Flag = false,_1Hz_Flag = false;
static int16_t Counter = 0;

void BMS_Timers_Init()
{
	/* Timer value set to 40ms i.e. interrupt will occur at every 40ms and makes the flag true in ISR */
	Timer_Init(TIMER_2,_40ms_PERIOD);
}

uint64_t Get_System_Time()
{
	return SysTickCounter;
}

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

//void Set_Servo_Position(uint8_t Servo_Channel,uint16_t Position)
//{
//	if(Position >= MIN_LIMIT && Position <= MAX_LIMIT)
//	{
//		Set_PWM_value(Servo_Channel,Position);
//	}
//}

//void TIM6_PeriodElapsedCallback()
//{
//	Counter++;
//	if(Counter >= 50)
//	{
//		_1Hz_Flag = true;
//		Counter = 0;
//	}
//}
//
//void TIM7_PeriodElapsedCallback()
//{
//	_10Hz_Flag = true;
//}
