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
#include <Hardware_Config.h>
#include <BMS_Timing.h>
#include <BMS_GPIOs.h>

/* Variable which becomes true as soon as MCU enters the sleep mode. So that after waking up from
 * sleep it can start from the point where it left off */
extern bool Sleep_Mode;
extern bool Wakeup_From_Sleep ;


void MCU_Sleep_Mode_Init();
void MCU_Enter_Sleep_Mode();
void MCU_Exit_Sleep_Mode();
void SystemClock_Decrease(void);
void Set_System_Clock_Frequency(void);


#endif /* POWER_MANAGEMENT_H_ */
