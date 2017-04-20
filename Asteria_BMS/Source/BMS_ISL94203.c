/*
 * BMS_ISL94203.c
 *
 *  Created on: 20-Jan-2017
 *      Author: NIKHIL
 */

#include "BMS_ISL94203.h"
#include "BMS_Timing.h"

uint8_t EEPROM_ENABLE_DATA[2] = {0x89,0x01};
uint8_t RAM_ENABLE_DATA[2] = {0x89,0x00};
uint8_t ISL_SLEEP_DATA[2] = {0x88,0x04};

void ISL94203_Init()
{
	I2C_Init(ISL_I2C,I2C_OWN_ADDRESS,I2C_100KHZ);
}

void ISL94203_EEPROM_Access_Enable()
{
	I2C_WriteData(ISL_I2C,ISL94203_ADDRESS,EEPROM_ENABLE_DATA,sizeof(EEPROM_ENABLE_DATA));
}

void ISL94203_RAM_Access_Enable()
{
	I2C_WriteData(ISL_I2C,ISL94203_ADDRESS,RAM_ENABLE_DATA,sizeof(RAM_ENABLE_DATA));
}

void Access_RAM_Data()
{

}

ISL_WriteStatus ISL94203_User_EEPROM_Write(uint8_t Memory_Address,uint8_t *Data_Ptr,uint8_t Data_Size)
{
	ISL94203_EEPROM_Access_Enable();
	Delay_Millis(1);
	if(Data_Size > EEPROM_PAGE_SIZE)
	{
		return WRITE_ERROR;
	}
	else
	{
		uint8_t Data[5];
		for(uint8_t Counter = 0; Counter < Data_Size; Counter++)
		{
			Data[0] = Memory_Address++;
			Data[1] = Data_Ptr[Counter];
			I2C_WriteData(ISL_I2C,ISL94203_ADDRESS,Data,2);
			Delay_Millis(EEPROM_WRITE_DELAY);
		}
		return WRITE_OK;
	}
	ISL94203_RAM_Access_Enable();
}


void ISL94203_User_EEPROM_Read(uint8_t Memory_Address,uint8_t *Buffer,uint8_t Data_Size)
{
	ISL94203_EEPROM_Access_Enable();

	I2C_WriteData(ISL_I2C,ISL94203_ADDRESS,&Memory_Address,1);

	Delay_Millis(4);

	I2C_ReadData(ISL_I2C,ISL94203_ADDRESS|0x01,Buffer,4);

	ISL94203_RAM_Access_Enable();
}

void ISL94203_Force_Sleep()
{
	I2C_WriteData(ISL_I2C,ISL94203_ADDRESS,ISL_SLEEP_DATA,sizeof(ISL_SLEEP_DATA));
}

void ISL94203_Read_Status_Register(uint8_t *Data)
{
	uint8_t Status_Reg_Address = 0x83;
	I2C_WriteData(ISL_I2C,ISL94203_ADDRESS,&Status_Reg_Address,1);
	I2C_ReadData(ISL_I2C,ISL94203_ADDRESS|0x01,Data,1);
	Delay_Millis(5);

}
