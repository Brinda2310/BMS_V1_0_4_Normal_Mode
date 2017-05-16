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

/* GPIO pins settings(STM32L4) for switch and LEDs */
#define BMS_SWITCH										(1 << 11)

#define LED_1											(1 << 3)
#define LED_2											(1 << 7)
#define LED_3											(1 << 6)
#define LED_4											LED_1
#define LED_5											(1 << 11)

enum LED_Patterns
{
	SOC = 0, SOH
};
enum Switch_Status
{
	NOT_PRESSED = 0, PRESSED
};

enum LED_Pattern
{
	ONE_LED_ON = 0, TWO_LEDs_ON,THREE_LEDs_ON,FOUR_LEDs_ON
};

/* Function prototypes */
void BMS_Switch_Init();
uint8_t BMS_Read_Switch_Status();
void BMS_Status_LEDs_Init();
void BMS_Show_LED_Pattern(uint8_t Battery_Capacity);

#endif /* BMS_GPIOS_H_ */
