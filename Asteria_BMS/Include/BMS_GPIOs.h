/*
 * BMS_GPIOs.h
 *
 *  Created on: 26-Apr-2017
 *      Author: NIKHIL
 */

#ifndef BMS_GPIOS_H_
#define BMS_GPIOS_H_

#include "GPIO_API.h"

/* LED port definitions as per the designed hardware */
#define LED1_PORT										GPIO_A
#define LED2_PORT										GPIO_B
#define LED3_PORT										GPIO_B
#define LED4_PORT										GPIO_B
#define LED5_PORT										GPIO_A
#define LED6_PORT										GPIO_B
#define BMS_SWITCH_PORT									GPIO_A

/* GPIO pins settings(STM32L4) for switch and LEDs */
#define LED_1											(1 << 0)
#define LED_2											(1 << 7)
#define LED_3											(1 << 6)
#define LED_4											(1 << 5)
#define LED_5											(1 << 2)
#define LED_6											(1 << 0)
#define BMS_SWITCH										(1 << 8)

/* Enums for patterns to be displayed on LEDs
 * @ SOC	: State of charge
 * @ SOH	: State of health
 * */
enum LED_Patterns
{
	SOC = 0, SOH
};

enum LED_Status
{
	HIDE_STATUS = 0,SHOW_STATUS
};
enum Switch_Status
{
	NOT_PRESSED = 0, PRESSED
};

/* Function prototypes */
void BMS_Switch_Init();
uint8_t BMS_Read_Switch_Status();
void BMS_Status_LEDs_Init();
void BMS_Status_LED_Toggle();
void BMS_Show_LED_Pattern(uint8_t Battery_Capacity,uint8_t Status);

#endif /* BMS_GPIOS_H_ */
