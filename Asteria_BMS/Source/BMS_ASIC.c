/*
 * BMS_ISL94203.c
 *
 *  Created on: 20-Jan-2017
 *      Author: NIKHIL
 */

#include <BMS_ASIC.h>
#include "BMS_Timing.h"

uint8_t EEPROM_ENABLE_DATA[2] = {0x89,0x01};
uint8_t RAM_ENABLE_DATA[2] = {0x89,0x00};
uint8_t ISL_SLEEP_DATA[2] = {0x88,0x04};

double Current_Amperes = 0.0, Previous_Amperes = 0.0,Total_Pack_Capacity = 0.0;
uint32_t Current_Time = 0,Previous_Time = 0;

static ISL_943203_Data BMS_Data;
BMS_Status_Flags Status_Flag;

void BMS_ASIC_Init()
{
	I2C_Init(BMS_I2C,I2C_OWN_ADDRESS,I2C_100KHZ);
}

static void BMS_EEPROM_Access_Enable()
{
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,EEPROM_ENABLE_DATA,sizeof(EEPROM_ENABLE_DATA));
}

static void BMS_RAM_Access_Enable()
{
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,RAM_ENABLE_DATA,sizeof(RAM_ENABLE_DATA));
}

uint8_t BMS_User_EEPROM_Write(uint8_t Memory_Address,uint8_t *Data_Ptr,uint8_t Data_Size)
{
	BMS_EEPROM_Access_Enable();
//	Delay_Millis(1);
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

static void Set_BMS_Status_Flags(uint32_t Flags)
{
	if(Flags & IS_ISL_IN_SLEEP)
	{
		Status_Flag.BMS_In_Sleep = YES;
#if DEBUG_OPTIONAL == ENABLE
		BMS_Debug_COM_Write_Data("In sleep mode\r",14);
#endif
	}
	else
	{
		Status_Flag.BMS_In_Sleep = NO;
#if DEBUG_OPTIONAL == ENABLE
		BMS_Debug_COM_Write_Data("Non-sleep\r",10);
#endif
	}

	if(Flags & IS_PACK_DISCHARGING)
	{
		BMS_Data.Charging_Discharging_Status = DISCHARGING;
		Status_Flag.Pack_Discharging = YES;
#if DEBUG_OPTIONAL == ENABLE
		BMS_Debug_COM_Write_Data("Discharging\r",14);
#endif
	}
	else
	{
		BMS_Data.Charging_Discharging_Status = LOW_POWER_CONSUMPTION;
		Status_Flag.Pack_Discharging = NO;
#if DEBUG_OPTIONAL == ENABLE
		BMS_Debug_COM_Write_Data("Low power mode\r\r",16);
#endif
	}
}
uint8_t Get_BMS_Charge_Discharge_Status()
{
	return BMS_Data.Charging_Discharging_Status;
}
/*
 * This function gives the status flags of various RAM registers
 */
void BMS_Read_RAM_Status_Register()
{
	typedef union
	{
		uint8_t Data[4];
		uint32_t Stat_Flags;
	}Bytes_to_integer;

	Bytes_to_integer RAM_Flags;

	uint8_t Register_Address = RAM_STATUS_REG_ADDRESS;
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Register_Address,1);
	I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,RAM_Flags.Data,4);
	Set_BMS_Status_Flags(RAM_Flags.Stat_Flags);
}

static void Convert_To_Cell_Voltages(uint8_t *Data)
{
	unsigned short  *Integers;
	Integers = (unsigned short*)Data;

	BMS_Data.Cell1_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell2_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell3_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell4_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell5_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell6_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell7_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell8_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);

}
/*
 * This function can give individual cell voltages inside the pack
 */
void BMS_Read_Cell_Voltages()
{
	uint8_t Cell_Voltages[NUMBER_OF_CELLS];

	uint8_t Register_Address = CELL_VOLTAGE_ADDR;
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Register_Address,1);
	/* Sequential read method of ISL is used to read all the cell voltages as they are in sequence in EEPROM */
	I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,Cell_Voltages,NUMBER_OF_CELLS);
	/* This function converts the read HEX values from ISL; convert them to integer and then does calculation
	 * to find the actual cell voltage */
	Convert_To_Cell_Voltages(Cell_Voltages);
}

void BMS_Estimate_Initial_Capacity(void)
{
	static uint8_t Counter = 0;
	if(_25Hz_Flag == true)
	{
		Counter++;
		BMS_Estimate_Capacity_Used();
		_25Hz_Flag = false;
	}
}

double Get_BMS_Initial_Capacity()
{
	return BMS_Data.Total_Pack_Capacity;
}

void BMS_Estimate_Capacity_Used()
{
	Current_Time = Get_System_Time();
	Current_Amperes = Get_BMS_Pack_Current();
	BMS_Data.Capacity_Used += ((Current_Amperes + Previous_Amperes)/2) * ((double)(Current_Time - Previous_Time)/3600000);
	Previous_Amperes = Current_Amperes;
	Previous_Time = Current_Time;
}

double Get_BMS_Capacity_Used()
{
	return BMS_Data.Capacity_Used;
}

/*
 * This function can give the pack current and pack voltage
 * The parameters that can be passed to this function are PACK_VOLTAGE and PACK_CURRENT
 */
void BMS_Read_Pack_Voltage()
{
	uint16_t Pack_Data;
	uint8_t Address = PACK_VOLTAGE_ADDR;

	I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Address,1);
	I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,(uint8_t*)&Pack_Data,2);

	BMS_Data.Pack_Voltage = ((uint16_t)(Pack_Data) * 1.8 * 32)/(4095);
}

void BMS_Read_Pack_Current()
{
	uint16_t Pack_Data;
	uint8_t Address = PACK_CURRENT_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*)&Pack_Data, 2);

	BMS_Data.Pack_Current = (((float)(Pack_Data) * 1.8) / (4095 * CURRENT_GAIN * SENSE_RESISTOR_VALUE));

}

void BMS_Read_Pack_Temperature()
{
	uint16_t Pack_Data;
	float Lcl_Temperature_Volts = 0.0;

	uint8_t Address = PACK_TEMPERATURE_ADDR;

	I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Address,1);
	I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,(uint8_t*)&Pack_Data,2);

	Lcl_Temperature_Volts = ((float)(Pack_Data) * 1.8)/(4095);
	BMS_Data.Pack_Temperature_Degress = (((Lcl_Temperature_Volts*1000)/(1.8527)) - 273.15);
}

float Get_Cell1_Voltage()
{
	return BMS_Data.Cell1_Voltage;
}

float Get_Cell2_Voltage()
{
	return BMS_Data.Cell2_Voltage;
}

float Get_Cell3_Voltage()
{
	return BMS_Data.Cell3_Voltage;
}

float Get_Cell4_Voltage()
{
	return BMS_Data.Cell4_Voltage;
}

float Get_Cell5_Voltage()
{
	return BMS_Data.Cell5_Voltage;
}

float Get_Cell6_Voltage()
{
	return BMS_Data.Cell6_Voltage;
}

float Get_Cell7_Voltage()
{
	return BMS_Data.Cell7_Voltage;
}

float Get_Cell8_Voltage()
{
	return BMS_Data.Cell8_Voltage;
}

float Get_BMS_Pack_Voltage()
{
	return BMS_Data.Pack_Voltage;
}

float Get_BMS_Pack_Current()
{
	/* Convert the ampere current to milli amperes by mutliplying by 1000 */
	return (BMS_Data.Pack_Current * 1000);
}

float Get_BMS_Pack_Temperature()
{
	return BMS_Data.Pack_Temperature_Degress;
}

