/*
 * BMS_GPIOs.h
 *
 *  Created on: 26-Apr-2017
 *      Author: NIKHIL
 */

#ifndef BMS_GPIOS_H_
#define BMS_GPIOS_H_

#include "GPIO_API.h"
#define LED1_PORT										GPIO_A
#define LED2_PORT										GPIO_B
#define LED3_PORT										GPIO_B
#define LED4_PORT										GPIO_B
#define LED5_PORT										GPIO_A

#define BMS_SWITCH										(1 << 11)

#define LED_1											(1 << 3)
#define LED_2											(1 << 7)
#define LED_3											(1 << 6)
#define LED_4											LED_1
#define LED_5											(1 << 11)

enum Switch_Status
{
	NOT_PRESSED = 0, PRESSED
};

void BMS_Switch_Init();
uint8_t BMS_Read_Switch_Status();
void BMS_Status_LEDs_Init();
void BMS_Show_LED_Status(uint8_t Battery_Status);


#endif /* BMS_GPIOS_H_ */
