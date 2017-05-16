/*
 * BMS_ASIC.c
 *
 *  Created on: 20-Jan-2017
 *      Author: NIKHIL
 */

#include <BMS_ASIC.h>
#include <BMS_Timing.h>

/* Data bytes to be sent over I2C to enable write to ERPROM */
uint8_t EEPROM_ENABLE_DATA[2] = {0x89,0x01};
/* Data bytes to be sent over I2C to enable write to RAM  */
uint8_t RAM_ENABLE_DATA[2] = {0x89,0x00};

/* Data bytes to be sent over I2C to force BMS IC to sleep mode */
uint8_t ISL_SLEEP_DATA[2] = {0x88,0x04};

double Current_Amperes = 0.0, Previous_Amperes = 0.0,Total_Pack_Capacity = 0.0;
uint32_t Current_Time = 0,Previous_Time = 0;

static ISL_943203_Data BMS_Data;
BMS_Status_Flags Status_Flag;

/* Function initializes the I2C communication between MCU and BMS IC */
void BMS_ASIC_Init()
{
	I2C_Init(BMS_I2C,I2C_OWN_ADDRESS,I2C_100KHZ);
}

/* Function to enable the access to the EERPOM section of BMS IC */
static void BMS_EEPROM_Access_Enable()
{
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,EEPROM_ENABLE_DATA,sizeof(EEPROM_ENABLE_DATA));
}

/* Function to enable the access to the RAM section of BMS IC */
static void BMS_RAM_Access_Enable()
{
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,RAM_ENABLE_DATA,sizeof(RAM_ENABLE_DATA));
}

/**
 * @brief  Function to write the user bytes to the EEPROM section of BMS IC
 * @param  Memory_Address	: EEPROM address to which data is to be written
 * @param  Data_Ptr			: Pointer to the data buffer which is to be written to the EEPROM locations
 * @param  Data_Size 		: Number of bytes to be written to the EEPROM locations
 * @retval WRITE_ERROR		: Not Successful
 * 		   WRITE_OK			: Successful
 */
uint8_t BMS_User_EEPROM_Write(uint8_t Memory_Address,uint8_t *Data_Ptr,uint8_t Data_Size)
{
	/* Before writing to EEPROM it's access needs to be enabled */
	BMS_EEPROM_Access_Enable();

	/* Make sure that data size is of 4 bytes only as BMS IC does not allow more than 4 bytes to write to
	 * it's user EEPROM */
	if(Data_Size > EEPROM_PAGE_SIZE)
	{
		return WRITE_ERROR;
	}
	else
	{
		/* Write the user data to the mentioned address; Delay of 30ms is required for each EEPROM byte write */
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
	/* Enable back the access to RAM as query data is always received from RAM locations */
	BMS_RAM_Access_Enable();
}

/**
 * @brief  Function to write the user bytes to the EEPROM section of BMS IC
 * @param  Memory_Address	: EEPROM address from which data is to be read
 * @param  Data_Ptr			: Pointer to the buffer to read the EEPROM data
 * @param  Data_Size 		: Number of bytes to be read from EEPROM locations
 * @retval WRITE_ERROR		: Not Successful
 * 		   WRITE_OK			: Successful
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

/* Function to set the respective flags defined in the code read from BMS status registers */
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

/* Function to return the charging/discharging status of the BMS */
uint8_t Get_BMS_Charge_Discharge_Status()
{
	return BMS_Data.Charging_Discharging_Status;
}

/* This function to query the status flags of various RAM registers */
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

/* Function to convert the cell voltages read from ISL registers(HEX) into float values */
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
/* Function to read individual cell voltages inside the pack */
void BMS_Read_Cell_Voltages()
{
	uint8_t Cell_Voltages[CELL_VOLTAGES_DATA_SIZE];

	uint8_t Register_Address = CELL_VOLTAGE_ADDR;
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Register_Address,1);
	/* Sequential read method of ISL is used to read all the cell voltages as they are in sequence in EEPROM */
	I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,Cell_Voltages,CELL_VOLTAGES_DATA_SIZE);
	/* This function converts the read HEX values from ISL; convert them to integer and then does calculation
	 * to find the actual cell voltage */
	Convert_To_Cell_Voltages(Cell_Voltages);
}

/* Function to calculate the battery capacity used */
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

/* Function to read the pack voltage from ISL IC */
void BMS_Read_Pack_Voltage()
{
	uint16_t Pack_Data;
	uint8_t Address = PACK_VOLTAGE_ADDR;

	I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Address,1);
	I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,(uint8_t*)&Pack_Data,2);

	BMS_Data.Pack_Voltage = ((uint16_t)(Pack_Data) * 1.8 * 32)/(4095);
}

/* Function to read the pack current going into or out of pack from ISL IC */
void BMS_Read_Pack_Current()
{
	uint16_t Pack_Data;
	uint8_t Address = PACK_CURRENT_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*)&Pack_Data, 2);

	BMS_Data.Pack_Current = (((float)(Pack_Data) * 1.8) / (4095 * CURRENT_GAIN * SENSE_RESISTOR_VALUE));
}

/* Function to read the pack temperature of pack from ISL IC */
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

/* All these function just return the values that are updated from BMS IC registers */
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
	/* Convert the ampere current to milli amperes by multiplying it by 1000 */
	return (BMS_Data.Pack_Current * 1000);
}

float Get_BMS_Pack_Temperature()
{
	return BMS_Data.Pack_Temperature_Degress;
}

