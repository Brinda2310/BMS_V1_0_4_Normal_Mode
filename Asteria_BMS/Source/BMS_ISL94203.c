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

/*
 * Function to read the user EEPROM data from ISL94203
 */
void ISL94203_User_EEPROM_Read(uint8_t Memory_Address,uint8_t *Buffer,uint8_t Data_Size)
{
	/* Enable the EEPROM access otherwise values will not be stored */
	ISL94203_EEPROM_Access_Enable();

	/* EEPROM location from which data is to be read */
	I2C_WriteData(ISL_I2C,ISL94203_ADDRESS,&Memory_Address,1);

	/* Read the number of bytes that from the EEPROM location */
	I2C_ReadData(ISL_I2C,ISL94203_ADDRESS|0x01,Buffer,4);

	/* Disable the EEPROM access to access the RAM */
	ISL94203_RAM_Access_Enable();
}

/*
 * Function to put ISL94203 into the sleep mode
 */
void ISL94203_Force_Sleep()
{
	I2C_WriteData(ISL_I2C,ISL94203_ADDRESS,ISL_SLEEP_DATA,sizeof(ISL_SLEEP_DATA));
}
/*
 * This function gives the status flags of various RAM registers
 * The parameters to this function should be
 * RAM_0x80_STATUS
 * RAM_0x81_STATUS
 * RAM_0x82_STATUS
 * RAM_0x83_STATUS
 */
void ISL94203_RAM_Status_Register(uint8_t Register_Address,uint8_t *Data)
{
	I2C_WriteData(ISL_I2C,ISL94203_ADDRESS,&Register_Address,1);
	I2C_ReadData(ISL_I2C,ISL94203_ADDRESS|0x01,Data,1);
}
/*
 * This function can give individual cell voltages inside the pack
 */
void Read_Cell_Voltages(uint8_t Register_Address,uint8_t *Data)
{
	I2C_WriteData(ISL_I2C,ISL94203_ADDRESS,&Register_Address,1);
	I2C_ReadData(ISL_I2C,ISL94203_ADDRESS|0x01,Data,2);
}

/*
 * This function can give the pack current and pack voltage
 * The parameters that can be passed to this function are PACK_VOLTAGE and PACK_CURRENT
 */
void Read_Pack_Data(uint8_t Register_Address,uint8_t *Data)
{
	I2C_WriteData(ISL_I2C,ISL94203_ADDRESS,&Register_Address,1);
	I2C_ReadData(ISL_I2C,ISL94203_ADDRESS|0x01,Data,2);
}
