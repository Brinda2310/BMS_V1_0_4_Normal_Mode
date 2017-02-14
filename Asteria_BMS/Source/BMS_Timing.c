/*
 * BMS_Timing.c
 *
 *  Created on: 17-Jan-2017
 *      Author: NIKHIL
 */

#include "BMS_Timing.h"
#include "stdbool.h"

bool _50Hz_Flag = false,_10Hz_Flag = false,_1Hz_Flag = false;
static int16_t Counter = 0;
void BMS_Timers_Init()
{
//	Timer_Init(TIMER_2,6000,_BOOT_TIMER_USE);
	Timer_Init(TIMER_6,20);
	Timer_Init(TIMER_7,100);
}

uint64_t Get_System_Time()
{
	return SysTickCounter;
}

void Set_Servo_Position(uint8_t Servo_Channel,uint16_t Position)
{
	if(Position >= MIN_LIMIT && Position <= MAX_LIMIT)
	{
		Set_PWM_value(Servo_Channel,Position);
	}
}
void TIM2_PeriodElapsedCallback()
{

}

void TIM6_PeriodElapsedCallback()
{
	_50Hz_Flag = true;
	Counter++;
	if(Counter >= 59)
	{
		_1Hz_Flag = true;
		Counter = 0;
	}
}

void TIM7_PeriodElapsedCallback()
{
	_10Hz_Flag = true;
}
