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
#define _1_SECONDS				(1000/_40ms_PERIOD)
#define _2_SECONDS				(2*_1_SECONDS)
#define _3_SECONDS				(3*_1_SECONDS)
#define _4_SECONDS				(4*_1_SECONDS)
#define _5_SECONDS				(5*_1_SECONDS)

#define LOW_CONSUMPTION_DELAY	(60*_1_SECONDS)
#define MCU_GO_TO_SLEEP_DELAY	LOW_CONSUMPTION_DELAY
#define SHORT_PERIOD			_2_SECONDS
#define LONG_PEROID				_4_SECONDS

extern bool _25Hz_Flag,_1Hz_Flag;

void BMS_Timers_Init();
uint64_t Get_System_Time();
void TIM2_PeriodElapsedCallback(void);

//void TIM6_PeriodElapsedCallback(void);
//void TIM7_PeriodElapsedCallback(void);
//
//void Set_Servo_Position(uint8_t Servo,uint16_t Position);

#endif /* BMS_TIMING_H_ */

