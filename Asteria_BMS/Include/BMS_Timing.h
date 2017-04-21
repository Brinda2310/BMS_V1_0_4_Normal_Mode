/*
 * BMS_Timing.h
 *
 *  Created on: 17-Jan-2017
 *      Author: NIKHIL
 */

#ifndef BMS_TIMING_H_
#define BMS_TIMING_H_

#include "TIMER_API.h"

#define _40ms_PERIOD 			40

extern bool _25Hz_Flag,_1Hz_Flag;

void BMS_Timers_Init();
uint64_t Get_System_Time();
void TIM2_PeriodElapsedCallback(void);

//void TIM6_PeriodElapsedCallback(void);
//void TIM7_PeriodElapsedCallback(void);
//
//void Set_Servo_Position(uint8_t Servo,uint16_t Position);

#endif /* BMS_TIMING_H_ */

