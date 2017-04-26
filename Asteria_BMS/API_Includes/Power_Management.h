/*
 * Power_Management.h
 *
 *  Created on: 07-Apr-2017
 *      Author: NIKHIL
 */

#ifndef POWER_MANAGEMENT_H_
#define POWER_MANAGEMENT_H_

#include <BMS_ASIC.h>
#include <BMS_Serial_Communication.h>
#include "Hardware_Config.h"
#include "BMS_Timing.h"
#include "GPIO_API.h"

void MCU_Sleep_Mode_Init();
void Enter_Sleep_Mode();
void Exit_Sleep_Mode();
void SystemPower_Config(void);
void SystemClock_Decrease(void);
void Set_System_Clock_Frequency(void);
void SystemClock_Config(void);


#endif /* POWER_MANAGEMENT_H_ */
