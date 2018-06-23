/*
 * BMS_Timing.h
 *
 *  Created on: 17-Jan-2017
 *      Author: NIKHIL
 */

#ifndef BMS_TIMING_H_
#define BMS_TIMING_H_

#include <TIMER_API.h>

extern bool flag;

#define _40ms_PERIOD_48MHz_SYS_Clock			24
#define _1sec_PERIOD_48MHz_SYS_Clock			600

#define _40ms_PERIOD_40MHz_SYS_Clock			20
#define _1sec_PERIOD_40MHz_SYS_Clock			500

#define _40ms_PERIOD_30MHz_SYS_Clock			15
#define _1sec_PERIOD_30MHz_SYS_Clock			375

#define _40ms_PERIOD_20MHz_SYS_Clock			10
#define _1sec_PERIOD_20MHz_SYS_Clock			250

#define _40ms_PERIOD_10MHz_SYS_Clock			5
#define _1sec_PERIOD_10MHz_SYS_Clock			125

/* period equation change */
#define _40ms_PERIOD_5MHz_SYS_Clock				25
#define _1sec_PERIOD_5MHz_SYS_Clock				625

#define _40ms_PERIOD_2MHz_SYS_Clock				10
#define _1sec_PERIOD_2MHz_SYS_Clock				250

#define _40ms_PERIOD_1MHz_SYS_Clock				5
#define _1sec_PERIOD_1MHz_SYS_Clock				125

#define _40ms_PERIOD 							40
#define _1_SECONDS								(1000/_40ms_PERIOD)
#define _2_SECONDS								(2*_1_SECONDS)
#define _3_SECONDS								(3*_1_SECONDS)
#define _4_SECONDS								(4*_1_SECONDS)
#define _5_SECONDS								(5*_1_SECONDS)
#define _10_SECONDS								(2* _5_SECONDS)
#define _20_SECONDS								(2* _10_SECONDS)

#define _500_MILLIS								(_1_SECONDS/2)
#define LOW_CONSUMPTION_DELAY					(60*_1_SECONDS)
#define LOW_CONSUMPTION_DELAY_AFTER_WAKEUP		(LOW_CONSUMPTION_DELAY/6)
#define CHARGE_TIME_DELAY						(1 * LOW_CONSUMPTION_DELAY)
#define DISCHARGE_TIME_DELAY					(1 * LOW_CONSUMPTION_DELAY)

#define MCU_GO_TO_SLEEP_DELAY					_5_SECONDS
#define SHORT_PERIOD							_500_MILLIS
#define LONG_PEROID								_2_SECONDS
#define DEBUG_FUNCTION_ENABLE_PERIOD			_5_SECONDS
#define FACTORY_DEFAULT_PEROID					_10_SECONDS

#define _1SEC_PERIOD							1000

extern bool _25Hz_Flag,_1Hz_Flag;

extern uint8_t Loop_Rate_Counter, Loop_Rate_Log_Counter;

/* BMS Idle time count variable increment every 1 second time interval in timer 6 ISR.*/
extern uint8_t BMS_Idle_Time_Count;

/* BMS Idle time count variable increment every 1 second time interval in timer 6 ISR.*/
extern uint8_t BMS_Doze_Time_Count;

/* Prototypes for the function defined in the BMS_Timing.c file */
void BMS_Timers_Init();
double Get_System_Time_Seconds();
uint64_t Get_System_Time_Millis();
void TIM2_PeriodElapsedCallback(void);
void TIM6_PeriodElapsedCallback(void);
//void TIM7_PeriodElapsedCallback(void);

#endif /* BMS_TIMING_H_ */

