
/******************************************************************************
  * @file    TIMER_API.h
  * @author  NIKHIL INGALE
  * @date    16-Jan-2017
  * @brief   This file contains all the functions prototypes for the TIMER
  *          module driver
  ******************************************************************************/
#ifndef TIMER_API_H_
#define TIMER_API_H_

#include "Hardware_Config.h"

#ifdef BMS_VERSION

void TIM2_PeriodElapsedCallback();
void TIM6_PeriodElapsedCallback();
void TIM7_PeriodElapsedCallback();

#endif

/* Number of Timers for stm32l432kc micro controller */
enum Timers
{
	TIMER_1 = 0,TIMER_2 , TIMER_6,TIMER_7,TIMER_15,TIMER_16
};

/* Number of PWM channels for stm32l432kc micro controller */
enum PWM_Channels
{
	TIM1_CHANNEL_1 = 0,TIM1_CHANNEL_2,TIM1_CHANNEL_3,TIM1_CHANNEL_4,
	TIM2_CHANNEL_1,TIM2_CHANNEL_2,TIM2_CHANNEL_3,TIM2_CHANNEL_4,
	TIM15_CHANNEL_1,TIM15_CHANNEL_2,
	TIM16_CHANNEL_1
};

//enum timer_modes
//{
//	_NORMAL_TIMER_USE = 0, _BOOT_TIMER_USE ,_PWM_OUTPUT,_PWM_INPUT
//};


/* Function Prototypes */
uint8_t Timer_Init(uint8_t Timer_Num,uint32_t Period);
uint8_t Timer_DeInit(uint8_t Timer_Num);
void Delay_Millis(uint32_t Delay_Value);
uint16_t Get_Current_Time_Count(uint8_t Timer_Num);
uint8_t PWM_Init(uint8_t Timer_Num,uint32_t PWM_Period);
uint8_t Set_PWM_value(uint8_t PWM_Channel_Num,uint16_t PWM_Value);

#endif /* TIMER_API_H_ */

