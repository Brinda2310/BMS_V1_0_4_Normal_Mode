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

void BMS_Init()
{
	I2C_Init(BMS_I2C,I2C_OWN_ADDRESS,I2C_100KHZ);
}

void BMS_EEPROM_Access_Enable()
{
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,EEPROM_ENABLE_DATA,sizeof(EEPROM_ENABLE_DATA));
}

void BMS_RAM_Access_Enable()
{
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,RAM_ENABLE_DATA,sizeof(RAM_ENABLE_DATA));
}

ISL_WriteStatus BMS_User_EEPROM_Write(uint8_t Memory_Address,uint8_t *Data_Ptr,uint8_t Data_Size)
{
	BMS_EEPROM_Access_Enable();
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
			I2C_WriteData(BMS_I2C,BMS_ADDRESS,Data,2);
			Delay_Millis(EEPROM_WRITE_DELAY);
		}
		return WRITE_OK;
	}
	BMS_RAM_Access_Enable();
}

/*
 * Function to read the user EEPROM data from ISL94203
 */
void BMS_User_EEPROM_Read(uint8_t Memory_Address,uint8_t *Buffer,uint8_t Data_Size)
{
	/* Enable the EEPROM access otherwise values will not be stored */
	BMS_EEPROM_Access_Enable();

	/* EEPROM location from which data is to be read */
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Memory_Address,1);

	/* Read the number of bytes that from the EEPROM location */
	I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,Buffer,4);

	/* Disable the EEPROM access to access the RAM */
	BMS_RAM_Access_Enable();
}

/*
 * Function to put ISL94203 into the sleep mode
 */
void BMS_Force_Sleep()
{
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,ISL_SLEEP_DATA,sizeof(ISL_SLEEP_DATA));
}
/*
 * This function gives the status flags of various RAM registers
 * The parameters to this function should be
 * RAM_0x80_STATUS
 * RAM_0x81_STATUS
 * RAM_0x82_STATUS
 * RAM_0x83_STATUS
 */
void BMS_RAM_Status_Register(uint8_t Register_Address,uint8_t *Data)
{
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Register_Address,1);
	I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,Data,1);
}
static void Convert_To_Cell_Voltages(uint8_t *Data)
{
//	char Buffer[50];
	uint16_t Cell1_V,Cell2_V,Cell3_V,Cell4_V,Cell5_V,Cell6_V,Cell7_V,Cell8_V;

	Cell1_V = (*Data++) | (*Data++ << 8);
	Cell2_V = (*Data++) | (*Data++ << 8);
	Cell3_V = (*Data++) | (*Data++ << 8);
	Cell4_V = (*Data++) | (*Data++ << 8);
	Cell5_V = (*Data++) | (*Data++ << 8);
	Cell6_V = (*Data++) | (*Data++ << 8);
	Cell7_V = (*Data++) | (*Data++ << 8);
	Cell8_V = (*Data++) | (*Data++ << 8);

	BMS_Data.Cell1_Voltage = (Cell1_V * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell2_Voltage = (Cell2_V * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell3_Voltage = (Cell3_V * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell4_Voltage = (Cell4_V * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell5_Voltage = (Cell5_V * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell6_Voltage = (Cell6_V * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell7_Voltage = (Cell7_V * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell8_Voltage = (Cell8_V * 1.8 * 8)/ (4095 * 3);

//	uint8_t Length = 0;
//	Length += sprintf(Buffer,"%0.3f\r",BMS_Data.Cell1_Voltage);
//	Length += sprintf(&Buffer[Length],"%0.3f\r",BMS_Data.Cell2_Voltage);
//	Length += sprintf(&Buffer[Length],"%0.3f\r",BMS_Data.Cell3_Voltage);
//	Length += sprintf(&Buffer[Length],"%0.3f\r",BMS_Data.Cell4_Voltage);
//	Length += sprintf(&Buffer[Length],"%0.3f\r",BMS_Data.Cell5_Voltage);
//	Length += sprintf(&Buffer[Length],"%0.3f\r",BMS_Data.Cell6_Voltage);
//	Length += sprintf(&Buffer[Length],"%0.3f\r",BMS_Data.Cell7_Voltage);
//	Length += sprintf(&Buffer[Length],"%0.3f\r",BMS_Data.Cell8_Voltage);
//
//	BMS_COM_Write_Data(Buffer,Length);
//	Delay_Millis(5);
}
/*
 * This function can give individual cell voltages inside the pack
 */
void BMS_Read_Cell_Voltages(uint8_t Register_Address,uint8_t *Received_Data)
{
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Register_Address,1);
	/* Sequential read method of ISL is used to read all the cell voltages as they are in sequence in EEPROM */
	I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,Received_Data,16);
	Delay_Millis(2);
	/* This function converts the read HEX values from ISL; convert them to integer and then does calculation
	 * to find the actual cell voltage */
	Convert_To_Cell_Voltages(Received_Data);
}

/*
 * This function can give the pack current and pack voltage
 * The parameters that can be passed to this function are PACK_VOLTAGE and PACK_CURRENT
 */
void BMS_Read_Pack_Data(uint8_t Register_Address)
{
	uint8_t Received_Data[2];

	I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Register_Address,1);
	I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,Received_Data,2);

	if(Register_Address == PACK_VOLTAGE)
	{
		uint16_t Converted_Pack_Voltage;
		Convert_Bytes_To_Short(Received_Data,&Converted_Pack_Voltage);
		BMS_Data.Pack_Voltage = (Converted_Pack_Voltage * 1.8 * 32)/(4095);
	}
	else if (Register_Address == PACK_CURRENT)
	{
		uint16_t Converted_Pack_Current;
		Convert_Bytes_To_Short(Received_Data,&Converted_Pack_Current);
		BMS_Data.Pack_Current = (((float)Converted_Pack_Current * 1800) / (4095 * CURRENT_GAIN * SENSE_RESISTOR_VALUE));
	}
}

void BMS_Read_Pack_Temperature()
{
	uint8_t Received_Data[2];
	uint8_t Register_Address = BMS_PACK_TEMPERATURE;
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Register_Address,1);
	I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,Received_Data,2);
	uint16_t Converted_Pack_Temperature = 0;
	Convert_Bytes_To_Short(Received_Data,&Converted_Pack_Temperature);
	BMS_Data.Pack_Temperature = ((float)Converted_Pack_Temperature * 1.8)/(4095);

}
void Convert_Bytes_To_Short(uint8_t *Data,uint16_t *Short_Data)
{
	*Short_Data = (*Data++) | ((*Data++) << 8);
}
