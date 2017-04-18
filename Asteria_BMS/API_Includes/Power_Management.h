/*
 * Power_Management.h
 *
 *  Created on: 07-Apr-2017
 *      Author: NIKHIL
 */

#ifndef POWER_MANAGEMENT_H_
#define POWER_MANAGEMENT_H_

#include "Hardware_Config.h"

void MCU_Sleep_Mode_Init();
void Enter_Sleep_Mode();
void SystemPower_Config(void);
void SystemClock_Decrease(void);
void Set_System_Clock_Frequency(void);


#endif /* POWER_MANAGEMENT_H_ */
