/*
 * BMS_GPIOs.c
 *
 *  Created on: 26-Apr-2017
 *      Author: NIKHIL
 */

#include "BMS_GPIOs.h"
#include <BMS_Serial_Communication.h>

void BMS_Switch_Init()
{
	GPIO_Init(GPIO_A,GPIO_PIN_11,GPIO_INPUT,PULLUP);
}

uint8_t BMS_Read_Switch_Status()
{
	if(GPIO_Read(GPIO_A,GPIO_PIN_11) == PIN_LOW)
	{
		return PRESSED;
	}
	else
	{
		return NOT_PRESSED;
	}
}

void BMS_Status_LEDs_Init()
{
	GPIO_Init(GPIO_B,LED_1,GPIO_OUTPUT,NOPULL);
//	GPIO_Init(GPIO_B,LED_2,GPIO_OUTPUT,NOPULL);
//	GPIO_Init(GPIO_B,LED_3,GPIO_OUTPUT,NOPULL);
//	GPIO_Init(GPIO_B,LED_4,GPIO_OUTPUT,NOPULL);
//	GPIO_Init(GPIO_A,LED_5,GPIO_OUTPUT,NOPULL);
}

void BMS_Show_LED_Pattern(uint8_t Battery_Capacity)
{
	if(Battery_Capacity == 1)
	{
		GPIO_Write(GPIO_B,LED_1,PIN_TOGGLE);
#if DEBUG_COM_WRITE == ENABLE
		BMS_Debug_COM_Write_Data("Short Press\r",12);
#endif
		Delay_Millis(2);
	}
	else if (Battery_Capacity == 2)
	{
#if DEBUG_COM_WRITE == ENABLE
		BMS_Debug_COM_Write_Data("Long Press\r",11);
#endif
	}
}
