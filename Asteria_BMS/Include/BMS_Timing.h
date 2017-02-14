/*
 * BMS_Timing.h
 *
 *  Created on: 17-Jan-2017
 *      Author: NIKHIL
 */

#ifndef BMS_TIMING_H_
#define BMS_TIMING_H_

#include "TIMER_API.h"

#define AILERON											TIM1_CHANNEL_1
#define RUDDER											TIM1_CHANNEL_2
#define ELEVATOR										TIM1_CHANNEL_3
#define THROTTLE										TIM1_CHANNEL_4

#define AILERON1										TIM2_CHANNEL_1
#define RUDDER1											TIM2_CHANNEL_2
#define ELEVATOR1										TIM2_CHANNEL_3
#define THROTTLE1										TIM2_CHANNEL_4

#define AILERON2										TIM15_CHANNEL_1
#define RUDDER2											TIM15_CHANNEL_2

#define AILERON3										TIM16_CHANNEL_1

#define MIN_LIMIT										1000
#define CENTER_POSITION									1500
#define MAX_LIMIT										2000

#define _20ms_PERIOD									20
#define _100ms_PERIOD									100
#define _6_SECONDS										6000

extern bool _50Hz_Flag,_10Hz_Flag,_1Hz_Flag;

void BMS_Timers_Init();
uint64_t Get_System_Time();
void TIM2_PeriodElapsedCallback(void);
void TIM6_PeriodElapsedCallback(void);
void TIM7_PeriodElapsedCallback(void);

void Set_Servo_Position(uint8_t Servo,uint16_t Position);

#endif /* BMS_TIMING_H_ */

