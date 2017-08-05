/*
 * AP_Communication.c
 *
 *  Created on: 02-May-2017
 *      Author: NIKHIL
 */
#include <AP_Communication.h>

bool Sleep_Mode_Funtionality = DISABLE;

AP_Status AP_Stat_Data;
uint8_t GPS_Data[20];
Pack_Info Pack_Data;

void BMS_Enable_Listen_Mode()
{
	SMBUS_Enable_Listen_Mode(BMS_SMBUS);
}

void BMS_Disable_Listen_Mode()
{
	SMBUS_Disable_Listen_Mode(BMS_SMBUS);
}

uint8_t AP_COM_Init(uint8_t Communication_Mode)
{
	uint8_t Result = 255;
	AP_COM_DeInit();
	Result = I2C_Init(BMS_SMBUS, BMS_SMBUS_OWN_ADDRESS, I2C_100KHZ, I2C_SLAVE);
	BMS_Enable_Listen_Mode();

	return Result;
}
uint8_t AP_COM_DeInit()
{
	uint8_t Result = 255;
	Result = I2C_DeInit(BMS_SMBUS);
	BMS_Disable_Listen_Mode();
	return Result;
}

