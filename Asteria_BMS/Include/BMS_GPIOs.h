/*
 * BMS_GPIOs.h
 *
 *  Created on: 26-Apr-2017
 *      Author: NIKHIL
 */

#ifndef BMS_GPIOS_H_
#define BMS_GPIOS_H_

#include "GPIO_API.h"

enum Switch_Status
{
	NOT_PRESSED = 0, PRESSED
};

void BMS_Switch_Init();
uint8_t BMS_Read_Switch_Status();
void BMS_Status_LEDs_Init();
void BMS_Show_LED_Status(uint8_t Battery_Status);


#endif /* BMS_GPIOS_H_ */
