/*
 * BMS_GPIOs.c
 *
 *  Created on: 26-Apr-2017
 *      Author: NIKHIL
 */

#include "BMS_GPIOs.h"

void BMS_Switch_Init()
{
	GPIO_Init(GPIO_A,BMS_SWITCH,GPIO_INPUT,PULLUP);
}

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

void BMS_Status_LEDs_Init()
{
	GPIO_Init(GPIO_A,LED_1,GPIO_OUTPUT,NOPULL);
//	GPIO_Init(GPIO_B,LED_2,GPIO_OUTPUT,NOPULL);
//	GPIO_Init(GPIO_B,LED_3,GPIO_OUTPUT,NOPULL);
	GPIO_Init(GPIO_B,LED_4,GPIO_OUTPUT,NOPULL);
	GPIO_Init(GPIO_A,LED_5,GPIO_OUTPUT,NOPULL);
}

void BMS_Show_LED_Status(uint8_t Battery_Status)
{
	GPIO_Write(GPIO_A,LED_1,PIN_TOGGLE);
}
