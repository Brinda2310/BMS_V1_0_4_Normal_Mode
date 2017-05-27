/*
 * BMS_GPIOs.c
 *
 *  Created on: 26-Apr-2017
 *      Author: NIKHIL
 */

#include <BMS_GPIOs.h>
#include <BMS_Serial_Communication.h>

/* Function to initialize the switch functionality connected to respective GPIO of MCU */
void BMS_Switch_Init()
{
	GPIO_Init(GPIO_A,BMS_SWITCH,GPIO_INPUT,PULLUP);
}

/* Function to read the switch status. Switch is pulled up with internal resistor,
 * If pressed pin status will be LOW and if not pressed it will remain to high state */
uint8_t BMS_Read_Switch_Status()
{
	if(GPIO_Read(GPIO_A,BMS_SWITCH) == PIN_LOW)
	{
		return PRESSED;
	}
	else
	{
		return NOT_PRESSED;
	}
}

/* Function to initialize the LEDs connected to respective GPIOs of MCU */
void BMS_Status_LEDs_Init()
{
	GPIO_Init(GPIO_B,LED_1,GPIO_OUTPUT,NOPULL);
//	GPIO_Init(GPIO_B,LED_2,GPIO_OUTPUT,NOPULL);
//	GPIO_Init(GPIO_B,LED_3,GPIO_OUTPUT,NOPULL);
//	GPIO_Init(GPIO_B,LED_4,GPIO_OUTPUT,NOPULL);
//	GPIO_Init(GPIO_A,LED_5,GPIO_OUTPUT,NOPULL);
}

/* Function to toggle the status LED connected on STM32L4 dev board; Used only for debugging */
#if DEBUG_STATUS_LED == ENABLE
void BMS_Status_LED_Toggle()
{
	GPIO_Write(GPIO_B,LED_1,PIN_TOGGLE);
}
#endif

/* Function to show the LED pattern upon switch press; It shows SOC and SOH based
 * on time for which switch is pressed */
void BMS_Show_LED_Pattern(uint8_t Battery_Capacity)
{
	if(Battery_Capacity == SOC)
	{
#if DEBUG_MANDATORY == ENABLE
		BMS_Debug_COM_Write_Data("Short Press\r",12);
		Delay_Millis(2);
#endif
	}
	else if (Battery_Capacity == SOH)
	{
#if DEBUG_MANDATORY == ENABLE
		BMS_Debug_COM_Write_Data("Long Press\r",11);
#endif
	}
}
