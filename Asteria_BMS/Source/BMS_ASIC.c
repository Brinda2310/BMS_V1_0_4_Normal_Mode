/*
 * BMS_ASIC.c
 *
 *  Created on: 20-Jan-2017
 *      Author: NIKHIL
 */

#include <BMS_ASIC.h>
#include <BMS_Timing.h>

/* Battery Parameters */
const uint8_t Battery_ID[] = "AA/BATT/001";
const uint8_t BMS_Board_Serial_Number[] = "AA___";

/* Data bytes to be sent over I2C to enable write to ERPROM */
uint8_t EEPROM_ENABLE_DATA[2] = {0x89,0x01};

/* Data bytes to be sent over I2C to enable write to RAM  */
uint8_t RAM_ENABLE_DATA[2] = {0x89,0x00};

/* Data bytes to be sent over I2C to force BMS IC to sleep mode */
uint8_t ISL_SLEEP_DATA[2] = {0x88,0x04};

/* Variable to calculate the discharge rate of pack */
double Current_Amperes = 0.0, Previous_Amperes = 0.0,Total_Pack_Capacity = 0.0;
uint32_t Current_Time = 0,Previous_Time = 0;

/* Object of the structure holding all the data variables in which queried data is stored and used in the code */
ISL_943203_Data BMS_Data;

/* Object of the structure which holds the value of all the status flags updated from ISL at 25Hz */
BMS_Status_Flags Status_Flag;

/* Object of the structure which holds all I2C errors occurred during read and write operation */
I2C_Errors I2C_Error_Flag;

/* Variable to decide whether last pack operation was charging or discharging or low power mode */
uint8_t Last_Charge_Disharge_Status = LOW_POWER_CONSUMPTION;

/* Variable to set the current gain value which code sets during power up. The same value is used in the current
 * calculation in various BMS functions defined in the code */
uint16_t Current_Gain = CURRENT_GAIN_5X;

/* Variable to calculate the rate at which battery is discharging(per seconds) */
double C_D_Accumulated_mAH = 0.0;

/* Variable to copy the data from status flags and same is used to log it in the SD card */
uint32_t Error_Check_Data = 0;

/* Variable to set restart the I2C if any problem occurs during read and write operation */
static bool BMS_Com_Restart = false;

/**
 * @brief  Function initializes the I2C communication between MCU and BMS IC
 * @param  None
 * @retval None
 */
void BMS_ASIC_Init(void)
{
	BMS_Com_Restart = false;

	if(I2C_Init(BMS_I2C,I2C_OWN_ADDRESS,I2C_100KHZ,I2C_MASTER) ==  RESULT_OK)
	{
		I2C_Error_Flag.I2C_Init_Flag = 0;
	}
	else
	{
		I2C_Error_Flag.I2C_Init_Flag = 1;
		BMS_Com_Restart = true;
	}
}

/**
 * @brief  Function to check if there is proper communication happening with BMS ASIC
 * @param  None
 * @retval HEALTH_NOT_OK : Problem with I2C read or write operation
 * 		   HEALTH_OK	 : I2C read/write operation is successful
 */
uint8_t BMS_Check_COM_Health()
{
	if(BMS_Com_Restart == true)
	{
		return HEALTH_NOT_OK;
	}
	else
	{
		return HEALTH_OK;
	}
}

/**
 * @brief  Function to enable the access to the EERPOM section of BMS IC
 * @param  None
 * @retval None
 */
static void BMS_EEPROM_Access_Enable()
{
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,EEPROM_ENABLE_DATA,sizeof(EEPROM_ENABLE_DATA));
}

/**
 * @brief  Function to enable the access to the RAM section of BMS IC
 * @param  None
 * @retval None
 */
static void BMS_RAM_Access_Enable()
{
	I2C_WriteData(BMS_I2C,BMS_ADDRESS,RAM_ENABLE_DATA,sizeof(RAM_ENABLE_DATA));
}

/**
 * @brief  Function to set the respective flags defined in the code which are read from BMS status registers
 * @param  Flags	: Actual status data received from BMS ASIC
 * @retval None
 */
static void BMS_Set_Status_Flags(uint32_t Flags)
{
	if(Flags & IS_ISL_IN_SLEEP)
	{
		Status_Flag.BMS_In_Sleep = YES;
	}
	else
	{
		Status_Flag.BMS_In_Sleep = NO;
	}

	if(Flags & IS_PACK_DISCHARGING)
	{
		BMS_Data.Charging_Discharging_Status = DISCHARGING;
		Status_Flag.Pack_Discharging = YES;
	}
	else
	{
		BMS_Data.Charging_Discharging_Status = LOW_POWER_CONSUMPTION;
		Status_Flag.Pack_Discharging = NO;
	}

	if(Flags & IS_PACK_CHARGING)
	{
		BMS_Data.Charging_Discharging_Status = CHARGING;
		Status_Flag.Pack_Charging = YES;
	}
	else
	{
		Status_Flag.Pack_Charging = NO;
	}
	if(Flags & IS_INTERNAL_SCAN)
	{
		Status_Flag.Internal_Scan_Progress = NO;
	}
	else
	{
		Status_Flag.Internal_Scan_Progress = YES;
	}
}

/**
 * @brief  Function to convert the cell voltages read from ISL registers(HEX) into float values
 * @param  Flags	: Pointer to the char data which is to be converted into the float
 * @retval None
 */
static void Convert_To_Cell_Voltages(uint8_t *Data)
{
	unsigned short  *Integers;
	Integers = (unsigned short*)&Data[0];

	/* Hard coded formulae defined by the ASIC manufacturer */
	BMS_Data.Cell1_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	Pack_Data.values[0] = BMS_Data.Cell1_Voltage;

	BMS_Data.Cell2_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	Pack_Data.values[1] = BMS_Data.Cell2_Voltage;

	BMS_Data.Cell3_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	Pack_Data.values[2] = BMS_Data.Cell3_Voltage;

	BMS_Data.Cell4_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell5_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);

	BMS_Data.Cell6_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	Pack_Data.values[3] = BMS_Data.Cell6_Voltage;

	BMS_Data.Cell7_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	Pack_Data.values[4] = BMS_Data.Cell7_Voltage;

	BMS_Data.Cell8_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	Pack_Data.values[5] = BMS_Data.Cell8_Voltage;
}

static void Convert_Float_Voltage_to_Hex(float Voltage_Value,uint8_t* Data_Buffer,uint8_t *Index)
{
	uint16_t Temp_Calculations = (uint16_t)(853.125 * Voltage_Value);

	Data_Buffer[(*Index)++] = (Temp_Calculations);
	Data_Buffer[(*Index)++] = ((Temp_Calculations >> 8)& 0xFF);
}

static void Convert_Degrees_Voltage_to_Hex(float Temperature_Degrees,uint8_t* Data_Buffer,uint8_t *Index)
{
	uint16_t  Temperature_in_Hex= 0.0;
	float Temperature_in_Volt = 0.0;
	Temperature_in_Volt = (Temperature_Degrees + 273.15) * 0.0018527;

	Temperature_in_Hex = (uint16_t)(2275 * Temperature_in_Volt);

	Data_Buffer[(*Index)++] = (Temperature_in_Hex);
	Data_Buffer[(*Index)++] = ((Temperature_in_Hex >> 8) & 0xFF);
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
	uint8_t Result;
	/* Before writing to EEPROM it's access needs to be enabled */
	BMS_EEPROM_Access_Enable();

	/* Make sure that data size is of 4 bytes only as BMS IC does not allow more than 4 bytes to write to
	 * it's user EEPROM */
	if(Data_Size > EEPROM_PAGE_SIZE)
	{
		Result = WRITE_ERROR;
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
		Result = WRITE_OK;
	}
	/* Enable back the access to RAM as query data is always received from RAM locations */
	BMS_RAM_Access_Enable();

	return Result;
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

/**
 * @brief  Function to put BMS ASIC into the sleep mode
 * @param  None
 * @retval None
 */
void BMS_Force_Sleep()
{
	if(I2C_WriteData(BMS_I2C,BMS_ADDRESS,ISL_SLEEP_DATA,sizeof(ISL_SLEEP_DATA)) == RESULT_OK)
	{
		I2C_Error_Flag.I2C_Force_Sleep = 0;
	}
	else
	{
		I2C_Error_Flag.I2C_Force_Sleep = 0;
	}
}

/**
 * @brief  Function to set the over voltage threshold value into BMS ASIC (configuration parameter)
 * @param  None
 * @retval None
 */
static void BMS_Set_Over_Voltage_Threshold(void)
{
	uint8_t *Index = 0,Data_Value[3];
	uint8_t Memory_Assign = 0;
	uint16_t Pack_Data = 0;

	Index = &Memory_Assign;

	/* Write the OV threshold value to the register. If any cell value is above threshold value then OV flag is
	 * set and logged on SD card */
	Data_Value[(*Index)++] = OV_THRESHOLD_ADDR;
	Convert_Float_Voltage_to_Hex(CELL_OVER_VOLTAGE_THR_VALUE,Data_Value,Index);
	I2C_WriteData(BMS_I2C, BMS_ADDRESS,Data_Value, *Index);

	/* Delay required between read and write operations for ISL */
	Delay_Millis(READ_WRITE_DELAY);

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	uint8_t Address = OV_THRESHOLD_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_ReadData(BMS_I2C,BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again  */
	uint16_t * Temp_Data = (uint16_t*)&Data_Value[1];
	if(Pack_Data == *Temp_Data)
	{
		I2C_Error_Flag.I2C_Set_OV_Thresh_Flag = 0;
	}
	else
	{
		I2C_Error_Flag.I2C_Set_OV_Thresh_Flag = 1;
	}
}

/**
 * @brief  Function to set the over voltage recovery value into BMS ASIC (configuration parameter)
 * @param  None
 * @retval None
 */
static void BMS_Set_Over_Voltage_Recovery(void)
{
	uint8_t *Index = 0,Data_Value[3];
	uint8_t Memory_Assign = 0;
	uint16_t Pack_Data = 0;

	Index = &Memory_Assign;

	/* Write the OV recovery value to the register. If any of the cell value comes below this value
	 * then OV flag is reseted and logged on SD card */
	Data_Value[(*Index)++] = OV_RECOVERY_ADDR;
	Convert_Float_Voltage_to_Hex(CELL_OV_RECOVERY_VALUE,Data_Value,Index);
	I2C_WriteData(BMS_I2C, BMS_ADDRESS,Data_Value, *Index);

	/* Delay required between read and write operations for ISL */
	Delay_Millis(READ_WRITE_DELAY);

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	uint8_t Address = OV_RECOVERY_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again */
	uint16_t * Temp_Data = (uint16_t*)&Data_Value[1];
	if(Pack_Data == *Temp_Data)
	{
		I2C_Error_Flag.I2C_Set_OV_Recovery_Flag = 0;
	}
	else
	{
		I2C_Error_Flag.I2C_Set_OV_Recovery_Flag = 1;
	}
}

/**
 * @brief  Function to set the under voltage threshold value into BMS ASIC (configuration parameter)
 * @param  None
 * @retval None
 */
static void BMS_Set_Under_Voltage_Threshold(void)
{
	uint8_t *Index = 0,Data_Value[3];
	uint8_t Memory_Assign = 0;
	uint16_t Pack_Data = 0;

	Index = &Memory_Assign;

	/* Write the UV threshold value to the register. If any cell value is below threshold value then UV flag
	 * is set and logged on SD card */
	Data_Value[(*Index)++] = UV_THROSHOLD_ADDR;
	Convert_Float_Voltage_to_Hex(CELL_UNDER_VOLTAGE_THR_VALUE,Data_Value,Index);
	I2C_WriteData(BMS_I2C, BMS_ADDRESS,Data_Value, *Index);

	/* Delay required between read and write operations for ISL */
	Delay_Millis(READ_WRITE_DELAY);

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	uint8_t Address = UV_THROSHOLD_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again */
	uint16_t * Temp_Data = (uint16_t*)&Data_Value[1];
	if(Pack_Data == *Temp_Data)
	{
		I2C_Error_Flag.I2C_Set_UV_Thresh_Flag = 0;
	}
	else
	{
		I2C_Error_Flag.I2C_Set_UV_Thresh_Flag = 1;
	}
}

/**
 * @brief  Function to set the under voltage recovery value into BMS ASIC (configuration parameter)
 * @param  None
 * @retval None
 */
static void BMS_Set_Under_Voltage_Recovery(void)
{
	uint8_t *Index = 0,Data_Value[3];
	uint8_t Memory_Assign = 0;
	uint16_t Pack_Data = 0;

	Index = &Memory_Assign;

	/* Write the UV recovery value to the register. If any cell value is above recovery value then UV flag
	 * is reseted and logged on SD card */
	Data_Value[(*Index)++] = UV_RECOVERY_ADDR;
	Convert_Float_Voltage_to_Hex(CELL_UV_RECOVERY_VALUE,Data_Value,Index);
	I2C_WriteData(BMS_I2C, BMS_ADDRESS,Data_Value, *Index);

	/* Delay required between read and write operations for ISL */
	Delay_Millis(READ_WRITE_DELAY);

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	uint8_t Address = UV_RECOVERY_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again */
	uint16_t * Temp_Data = (uint16_t*)&Data_Value[1];
	if(Pack_Data == *Temp_Data)
	{
		I2C_Error_Flag.I2C_Set_UV_Recovery_Flag = 0;
	}
	else
	{
		I2C_Error_Flag.I2C_Set_UV_Recovery_Flag = 1;
	}
}

/**
 * @brief  Function to set the OV lockout value into BMS ASIC (configuration parameter)
 * @param  None
 * @retval None
 */
static void BMS_Set_OV_LockOut_Threshold(void)
{
	uint8_t *Index = 0,Data_Value[3];
	uint8_t Memory_Assign = 0;
	uint16_t Pack_Data = 0;

	Index = &Memory_Assign;

	/* Write the OV lockout threshold value to the register. If any if the cell voltage is above this value
	 * for more than 5 successive scans then OVLO flag is set and logged on SD card */
	Data_Value[(*Index)++] = OV_LOCKOUT_THRESHOLD_ADDR;
	Convert_Float_Voltage_to_Hex(CELL_OV_LOCKOUT_THR_VALUE,Data_Value,Index);
	I2C_WriteData(BMS_I2C, BMS_ADDRESS,Data_Value, *Index);

	/* Delay required between read and write operations for ISL */
	Delay_Millis(READ_WRITE_DELAY);

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	uint8_t Address = OV_LOCKOUT_THRESHOLD_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again */
	uint16_t * Temp_Data = (uint16_t*)&Data_Value[1];
	if(Pack_Data == *Temp_Data)
	{
		I2C_Error_Flag.I2C_Set_OV_Lockout_Thresh_Flag = 0;
	}
	else
	{
		I2C_Error_Flag.I2C_Set_OV_Lockout_Thresh_Flag = 1;
	}
}

/**
 * @brief  Function to set the UV lockout value into BMS ASIC (configuration parameter)
 * @param  None
 * @retval None
 */
static void BMS_Set_UV_LockOut_Threshold(void)
{
	uint8_t *Index = 0,Data_Value[3];
	uint8_t Memory_Assign = 0;
	uint16_t Pack_Data = 0;

	Index = &Memory_Assign;

	/* Write the UV lockout threshold value to the register. If any if the cell voltage is below this value
	 * for more than 5 successive scans then UVLO flag is set and logged on SD card */
	Data_Value[(*Index)++] = UV_LOCKOUT_THRESHOLD_ADDR;
	Convert_Float_Voltage_to_Hex(CELL_UV_LOCKOUT_THR_VALUE,Data_Value,Index);
	I2C_WriteData(BMS_I2C, BMS_ADDRESS,Data_Value, *Index);

	/* Delay required between read and write operations for ISL */
	Delay_Millis(READ_WRITE_DELAY);

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	uint8_t Address = UV_LOCKOUT_THRESHOLD_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again */
	uint16_t * Temp_Data = (uint16_t*)&Data_Value[1];
	if(Pack_Data == *Temp_Data)
	{
		I2C_Error_Flag.I2C_Set_UV_Lockout_Thresh_Flag = 0;
	}
	else
	{
		I2C_Error_Flag.I2C_Set_UV_Lockout_Thresh_Flag = 1;
	}
}

/**
 * @brief  Function to set the end of charge threshold value into BMS ASIC (configuration parameter)
 * @param  None
 * @retval None
 */
static void BMS_Set_End_of_Charge_Threshold(void)
{
	uint8_t *Index = 0,Data_Value[3];
	uint8_t Memory_Assign = 0;
	uint16_t Pack_Data = 0;

	Index = &Memory_Assign;

	/* Write the EOC threshold value to the register. If any if the cell voltage is above this value
	 * EOC flag is set and logged on SD card */
	Data_Value[(*Index)++] = EOC_THRESHOLD_ADDR;
	Convert_Float_Voltage_to_Hex(CELL_EOC_THR_VALUE,Data_Value,Index);
	I2C_WriteData(BMS_I2C, BMS_ADDRESS,Data_Value, *Index);

	/* Delay required between read and write operations for ISL */
	Delay_Millis(READ_WRITE_DELAY);

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	uint8_t Address = EOC_THRESHOLD_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again */
	uint16_t * Temp_Data = (uint16_t*)&Data_Value[1];
	if(Pack_Data == *Temp_Data)
	{
		I2C_Error_Flag.I2C_Set_EOC_Thresh_Flag = 0;
	}
	else
	{
		I2C_Error_Flag.I2C_Set_EOC_Thresh_Flag = 1;
	}
}

static void BMS_Set_OV_Delay_Timeout(void)
{
	uint8_t *Index = 0,Data_Value[3];
	uint8_t Memory_Assign = 0;
	uint16_t Pack_Data = 0;

	Index = &Memory_Assign;

	/* Write the over voltage delay timeout value to the register. If any if the cell voltage is above over voltage threshold value for
	 * this much time period then set the OV flag and the same is logged on SD card */
	Data_Value[(*Index)++] = OV_DELAY_TIMEOUT_ADDR;
	Data_Value[(*Index)++] = OV_DELAY_TIMEOUT_VALUE;
	Data_Value[(*Index)++] = OV_DELAY_TIMEOUT_RESOLUTION | ((OV_DELAY_TIMEOUT_VALUE >> 8) & 0x03);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,Data_Value, *Index);

	/* Delay required between read and write operations for ISL */
	Delay_Millis(READ_WRITE_DELAY);

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	uint8_t Address = OV_DELAY_TIMEOUT_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again */
	uint16_t * Temp_Data = (uint16_t*)&Data_Value[1];
	if(Pack_Data == *Temp_Data)
	{
		I2C_Error_Flag.I2C_Set_OV_Delay_Timeout_Flag = 0;
	}
	else
	{
		I2C_Error_Flag.I2C_Set_OV_Delay_Timeout_Flag = 1;
	}
}

static void BMS_Set_UV_Delay_Timeout(void)
{
	uint8_t *Index = 0,Data_Value[3];
	uint8_t Memory_Assign = 0;
	uint16_t Pack_Data = 0;

	Index = &Memory_Assign;

	/* Write the under voltage delay timeout value to the register. If any if the cell voltage is below under voltage threshold value for
	 * this much time period then set the UV flag and the same is logged on SD card */
	Data_Value[(*Index)++] = UV_DELAY_TIMEOUT_ADDR;
	Data_Value[(*Index)++] = UV_DELAY_TIMEOUT_VALUE;
	Data_Value[(*Index)++] = UV_DELAY_TIMEOUT_RESOLUTION | ((UV_DELAY_TIMEOUT_VALUE >> 8) & 0x03);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,Data_Value, *Index);

	/* Delay required between read and write operations for ISL */
	Delay_Millis(READ_WRITE_DELAY);

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	uint8_t Address = UV_DELAY_TIMEOUT_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again */
	uint16_t * Temp_Data = (uint16_t*)&Data_Value[1];
	if(Pack_Data == *Temp_Data)
	{
		I2C_Error_Flag.I2C_Set_UV_Delay_Timeout_Flag = 0;
	}
	else
	{
		I2C_Error_Flag.I2C_Set_UV_Delay_Timeout_Flag = 1;
	}
}

static void BMS_Set_Open_Wiring_Timeout(void)
{
	uint8_t *Index = 0,Data_Value[3];
	uint8_t Memory_Assign = 0;
	uint16_t Pack_Data = 0;

	Index = &Memory_Assign;

	/* Write the open wiring timeout value to the register. If open circuit is detected for this much time period then
	 * OPEN flag is set and the same is logged on SD card */
	Data_Value[(*Index)++] = OPEN_WIRING_TIMEOUT_ADDR;
	Data_Value[(*Index)++] = OPEN_WIRING_TIMEOUT_VALUE;
	Data_Value[(*Index)++] = OPEN_WIRING_TIMEOUT_RESOLUTION | ((UV_DELAY_TIMEOUT_VALUE >> 8) & 0x01);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,Data_Value, *Index);

	/* Delay required between read and write operations for ISL */
	Delay_Millis(READ_WRITE_DELAY);

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	uint8_t Address = OPEN_WIRING_TIMEOUT_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again */
	uint16_t * Temp_Data = (uint16_t*)&Data_Value[1];
	if(Pack_Data == *Temp_Data)
	{
		I2C_Error_Flag.I2C_Set_Open_Wiring_Timeout_Flag= 0;
	}
	else
	{
		I2C_Error_Flag.I2C_Set_Open_Wiring_Timeout_Flag = 1;
	}
}

/**
 * @brief  Function to set internal OT threshold value into BMS ASIC (configuration parameter)
 * @param  None
 * @retval None
 */
static void BMS_Set_Internal_OT_Threshold(void)
{
	uint8_t *Index = 0,Data_Value[3];
	uint8_t Memory_Assign = 0;
	uint16_t Pack_Data = 0;

	Index = &Memory_Assign;

	/* Write the Internal over temperature threshold value to the register. If internal temperature is
	 * above this value then IOT flag is set and logged on SD card */
	Data_Value[(*Index)++] = INTERNAL_OT_THRESHOLD_ADDR;
	Convert_Degrees_Voltage_to_Hex(INTERNAL_OVER_TEMP_THR_VALUE, Data_Value, Index);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS, Data_Value, *Index);

	/* Delay required between read and write operations for ISL */
	Delay_Millis(READ_WRITE_DELAY);

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	uint8_t Address = INTERNAL_OT_THRESHOLD_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS, &Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again */
	uint16_t * Temp_Data = (uint16_t*) &Data_Value[1];
	if (Pack_Data == *Temp_Data)
	{
		I2C_Error_Flag.I2C_Set_IOT_Thresh_Flag = 0;
	}
	else
	{
		I2C_Error_Flag.I2C_Set_IOT_Thresh_Flag = 1;
	}
}

/**
 * @brief  Function to set the internal over temperature recovery value into BMS ASIC (configuration parameter)
 * @param  None
 * @retval None
 */
static void BMS_Set_Internal_OT_Recovery(void)
{
	uint8_t *Index = 0,Data_Value[3];
	uint8_t Memory_Assign = 0;
	uint16_t Pack_Data = 0;

	Index = &Memory_Assign;

	/* Write the Internal over temperature threshold value to the register. If internal temperature is
	 * above this value then IOT flag is set and logged on SD card */
	Data_Value[(*Index)++] = INTERNAL_OT_RECOVERY_ADDR;
	Convert_Degrees_Voltage_to_Hex(INTERNAL_OT_RECOVERY_VALUE, Data_Value, Index);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS, Data_Value, *Index);

	/* Delay required between read and write operations for ISL */
	Delay_Millis(READ_WRITE_DELAY);

	uint8_t Address = INTERNAL_OT_RECOVERY_ADDR;

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again */
	if (I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2) == RESULT_OK)
	{
		uint16_t * Temp_Data = (uint16_t*) &Data_Value[1];
		if (Pack_Data == *Temp_Data)
		{
			I2C_Error_Flag.I2C_Set_IOT_Recovery_Flag = 0;
		}
		else
		{
			I2C_Error_Flag.I2C_Set_IOT_Recovery_Flag = 1;
		}
	}
}

/**
 * @brief  Function to disable the cell balancing by external micro controller (configuration parameter)
 * @param  None
 * @retval None
 */
static void BMS_Disable_Cell_Balancing(void)
{
	uint8_t Data_Value[3],Index = 0;
	uint16_t Pack_Data = 0;

	/* Write the value to the register to disable the cell balancing; BMS ASIC does the cell balancing */
	Data_Value[Index++] = DISABLE_CELL_BALANCE_ADDR;
	Data_Value[Index++] = 0x00;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS, Data_Value, Index);

	uint8_t Address = DISABLE_CELL_BALANCE_ADDR;

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again */
	if (I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 1) == RESULT_OK)
	{
		uint8_t *Temp_Data = &Data_Value[1];
		if (Pack_Data == *Temp_Data)
		{
			I2C_Error_Flag.I2C_Disable_Cell_Balancing_Flag = 0;
		}
		else
		{
			I2C_Error_Flag.I2C_Disable_Cell_Balancing_Flag = 1;
		}
	}
}

/**
 * @brief  Function to disable the cell balancing by external micro controller (configuration parameter)
 * @param  None
 * @retval None
 */
static uint8_t BMS_Config_Number_of_Cells(uint8_t Num_of_Cells,uint8_t Operation)
{
	uint8_t Data_Value[3],Index = 0;
	uint16_t Cell_Info = 0;

	/* Write the value to the register to disable the cell balancing; BMS ASIC does the cell balancing */
	Data_Value[Index++] = NUMBER_OF_CELLS_ADDR;
	Data_Value[Index++] = 0xFF;

	if (Num_of_Cells == 8)
	{
		Data_Value[Index++] = 0xFF;
	}
	else if (Num_of_Cells == 6)
	{
		Data_Value[Index++] = 0xE7;
	}
	else if (Num_of_Cells == 4)
	{
		Data_Value[Index++] = 0xC3;
	}
	if (Operation == WRITE_REGISTER)
	{
		I2C_WriteData(BMS_I2C, BMS_ADDRESS, Data_Value, Index);
	}
	uint8_t Address = NUMBER_OF_CELLS_ADDR;

	/* Re-confirm whether threshold value is written to the register properly or not by reading the same register */
	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);

	/* If written value in BMS ASIC register and values read from the same register are same
	 * then configuration settings are OK other wise code should write the values to the register again */
	if (I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Cell_Info, 2) == RESULT_OK)
	{
		uint16_t *Temp_Data = (uint16_t*)&Data_Value[1];
		if (Cell_Info == *Temp_Data)
		{
			I2C_Error_Flag.I2C_Config_Number_of_Cells_Flag = 0;
			return Num_of_Cells;
		}
		else
		{
			I2C_Error_Flag.I2C_Config_Number_of_Cells_Flag = 1;
			return 0;
		}
	}
	else
		return 0;
}

/**
 * @brief  Function to set all the configuration parameters in the BMS ASIC
 * @param  None
 * @retval RESULT_OK	: Operation is successful
 * 		   RESULT_ERROR	: Operation is unsuccessful
 */
uint8_t BMS_Configure_Parameters(void)
{
	BMS_Set_Over_Voltage_Threshold();
	BMS_Set_Over_Voltage_Recovery();
	BMS_Set_Under_Voltage_Threshold();
	BMS_Set_Under_Voltage_Recovery();
	BMS_Set_OV_LockOut_Threshold();
	BMS_Set_UV_LockOut_Threshold();
	BMS_Set_End_of_Charge_Threshold();
	BMS_Set_OV_Delay_Timeout();
	BMS_Set_UV_Delay_Timeout();
	BMS_Set_Open_Wiring_Timeout();
	BMS_Set_Internal_OT_Threshold();
	BMS_Disable_Cell_Balancing();
	BMS_Set_Internal_OT_Recovery();
	BMS_Config_Number_of_Cells(BATT_NUMBER_OF_CELLS,WRITE_REGISTER);

	uint32_t *Temp_Data = (uint32_t*)&I2C_Error_Flag;
	if(*Temp_Data == 0x00)
	{
		BMS_Debug_COM_Write_Data("BMS Configuration Setting OK...!!!\r",35);
		return RESULT_OK;
	}
	else
	{
		BMS_Debug_COM_Write_Data("BMS Configuration Setting Failed...!!!\r",40);
		return RESULT_ERROR;
	}
}

/**
 * @brief  Function to set all the configuration parameters in the BMS ASIC
 * @param  None
 * @retval RESULT_OK	: Operation is successful
 * 		   RESULT_ERROR	: Operation is unsuccessful
 */
uint8_t BMS_Read_Number_Of_Cells_Configuration()
{
	if(BMS_Config_Number_of_Cells(BATT_NUMBER_OF_CELLS,READ_REGISTER) != BATT_NUMBER_OF_CELLS)
	{
		return RESULT_ERROR;
	}
	return RESULT_OK;
}

/**
 * @brief  Function to set the current gain value to the EEPROM of BMS ASIC
 * @param  None
 * @retval WRITE_OK		: I2C write is successful
 * 		   WRITE_ERROR	: I2C write is failed
 */
void BMS_Set_Current_Gain(uint16_t Gain_Setting)
{
	uint8_t Gain_Value;
	uint8_t Register_Address = 0, Send_Data_Values[2];
	Current_Gain = Gain_Setting;

	/* Before writing any value,make sure that other settings in the 0x85 register are not disturbed
	 * So read the value first and then just change the 4th and 5th bit value in the register */
	Register_Address = CURRENT_GAIN_SETTING_ADDR;
	I2C_WriteData(BMS_I2C, BMS_ADDRESS, &Register_Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, &Gain_Value, 1);

	if (Status_Flag.Internal_Scan_Progress == NO)
	{
		switch (Gain_Setting)
		{
		case CURRENT_GAIN_5X:
			Gain_Value = Gain_Value | (1 << 4);
			Gain_Value &= (~(1 << 5)); /* 4th bit should be 1 and 5th bit should be 0 */
			break;
		case CURRENT_GAIN_50X:
			Gain_Value = Gain_Value & (~(3 << 4)); /* 4th and 5th bit should be zero */
			break;
		case CURRENT_GAIN_500X:
			Gain_Value = Gain_Value | (1 << 5); /* 5th bit should be 1 and 4th bit can be either 0 or 1 */
			break;
		default:
			Gain_Value = Gain_Value | (1 << 4); /* By default gain value should be 5X*/
			Gain_Value &= (~(1 << 5));
			break;
		}
		Send_Data_Values[0] = CURRENT_GAIN_SETTING_ADDR;
		Send_Data_Values[1] = Gain_Value;
		/* Write the current gain value(5X,50X,500X) to 0x85 register */
		I2C_WriteData(BMS_I2C, BMS_ADDRESS, Send_Data_Values,sizeof(Send_Data_Values));
	}

	/* Re-confirm the value written to the register by reading it once */
	I2C_WriteData(BMS_I2C, BMS_ADDRESS, &Register_Address, 1);
	I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, &Gain_Value, 1);

	if(Gain_Value == Gain_Setting)
	{
		I2C_Error_Flag.I2C_Set_Current_Gain_Flag = 0;
		BMS_Com_Restart = false;
	}
	else
	{
		I2C_Error_Flag.I2C_Set_Current_Gain_Flag = 1;
		BMS_Com_Restart = true;
	}
}

/**
 * @brief  Function to set the maximum of charge/discharge pack cycles
 * @param  None
 * @retval None
 */
void BMS_Update_Pack_Cycles()
{
	if(BMS_Data.Pack_Charge_Cycles > BMS_Data.Pack_Discharge_Cycles)
	{
		BMS_Data.Pack_Total_Cycles = BMS_Data.Pack_Charge_Cycles;
	}
	else
	{
		BMS_Data.Pack_Total_Cycles = BMS_Data.Pack_Discharge_Cycles;
	}
}

/**
 * @brief  This function to query the status flags of various RAM registers
 * @param  None
 * @retval None
 */
void BMS_Read_RAM_Status_Register()
{
	typedef union
	{
		uint8_t Data[4];
		uint32_t Stat_Flags;
	}Bytes_to_integer;

	Bytes_to_integer RAM_Flags;

	uint8_t Register_Address = RAM_STATUS_REG_ADDR;
	if(I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Register_Address,1) == RESULT_OK)
	{
		if(I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,RAM_Flags.Data,4) == RESULT_OK)
		{
			I2C_Error_Flag.I2C_Read_Status_Flag = 0;
			BMS_Com_Restart = false;
		}
		else
		{
			I2C_Error_Flag.I2C_Read_Status_Flag = 1;
			BMS_Com_Restart = true;
		}
	}
	else
	{
		I2C_Error_Flag.I2C_Read_Status_Flag = 1;
		BMS_Com_Restart = true;
	}

	/* Save the copy of error flags read from the ISL94203 so that same can be logged on SD card */
	Error_Check_Data = RAM_Flags.Stat_Flags;

	BMS_Set_Status_Flags(RAM_Flags.Stat_Flags);
}

/**
 * @brief  Function to read individual cell voltages inside the pack
 * @param  None
 * @retval None
 */
void BMS_Read_Cell_Voltages()
{
	uint8_t Cell_Voltages[CELL_VOLTAGES_DATA_SIZE];

	uint8_t Register_Address = CELL_VOLTAGE_ADDR;

	/* Sequential read method of ISL is used to read all the cell voltages as they are in sequence in EEPROM */
	if(I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Register_Address,1) == RESULT_OK)
	{
		if(I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,Cell_Voltages,CELL_VOLTAGES_DATA_SIZE) == RESULT_OK)
		{
			I2C_Error_Flag.I2C_Read_Cells_Flag = 0;
			BMS_Com_Restart = false;
		}
		else
		{
			I2C_Error_Flag.I2C_Read_Cells_Flag = 1;
			BMS_Com_Restart = true;;
		}
	}
	else
	{
		I2C_Error_Flag.I2C_Read_Cells_Flag = 1;
		BMS_Com_Restart = true;;
	}

	/* This function converts the read HEX values from ISL; convert them to integer and then does calculation
	 * to find the actual cell voltage */
	Convert_To_Cell_Voltages(&Cell_Voltages[0]);
}

/**
 * @brief  Function to calculate the battery capacity used and remaining
 * @param  None
 * @retval None
 */
void BMS_Estimate_Initial_Capacity(void)
{
	float Batt_Volt_Per_Cell = 0, volt_z = 0, pow_volt_z = 0, Battery_Estimate = 0;

	Batt_Volt_Per_Cell = Get_BMS_Pack_Voltage() / (float) BATT_NUMBER_OF_CELLS;

	volt_z = (float) ((float) (Batt_Volt_Per_Cell - BATT_EST_Mu) / (float) BATT_EST_Sigma);

	pow_volt_z = volt_z; 				/* Degree 1 */

	Battery_Estimate = (float) BATT_EST_COEFF_0	+ (float) BATT_EST_COEFF_1 * (float) pow_volt_z;

	pow_volt_z = pow_volt_z * volt_z; 	/* Degree 2 */

	Battery_Estimate = Battery_Estimate + (float) BATT_EST_COEFF_2 * (float) pow_volt_z;

	pow_volt_z = pow_volt_z * volt_z; 	/* Degree 3 */

	Battery_Estimate = Battery_Estimate + (float) BATT_EST_COEFF_3 * (float) pow_volt_z;

	pow_volt_z = pow_volt_z * volt_z; 	/* Degree 4 */

	Battery_Estimate = Battery_Estimate + (float) BATT_EST_COEFF_4 * (float) pow_volt_z;

	pow_volt_z = pow_volt_z * volt_z; 	/* Degree 5 */

	Battery_Estimate = Battery_Estimate + (float) BATT_EST_COEFF_5 * (float) pow_volt_z;

	if ((Battery_Estimate > 100))
	{
		BMS_Data.Pack_Capacity_Used = 0;
	}
	else if (Battery_Estimate < 0)
	{
		BMS_Data.Pack_Capacity_Used = (float) (BATTERY_CAPACITY);
	}
	else
	{
		BMS_Data.Pack_Capacity_Used = ((1 - (Battery_Estimate / 100)) * (float)(BATTERY_CAPACITY));
	}

	/*	Calculate remaining battery capacity */
	BMS_Data.Pack_Capacity_Remaining = Battery_Estimate;//(float) ((float) (1.0 - (float) (BMS_Data.Pack_Capacity_Used	/ (float) (BATTERY_CAPACITY))) * 100);
	BMS_Data.Pack_Capacity_Remaining = Constrain(BMS_Data.Pack_Capacity_Remaining, 0, 100);

//	char Buffer[80];
//	uint8_t Length = sprintf(Buffer,"Batt Rem = %0.3fmA\r",BMS_Data.Pack_Capacity_Remaining);
//	Length += sprintf(&Buffer[Length],"Batt Used = %0.3fmA\r",BMS_Data.Pack_Capacity_Used);
//	BMS_Debug_COM_Write_Data(Buffer,Length);
}

/**
 * @brief  Function to calculate the pack capacity used over the time(dt = 40ms)
 * @param  None
 * @retval None
 */
void BMS_Estimate_Capacity_Used()
{
	static uint8_t Previous_State = LOW_POWER_CONSUMPTION;

	Current_Time = Get_System_Time_Millis();

	/* Default value for the current is in between 50-200mA. For board 1 it is varying in between 85-150mA (raw current)
	 * For board 3 it is giving 0mA (raw current) */
	if(BMS_Data.Pack_Current_Adjusted > 200.0f)
	{
		Current_Amperes = BMS_Data.Pack_Current_Adjusted;
	}
	else
	{
		Current_Amperes = 0.0;
	}
	BMS_Data.Pack_Charge_Discharge_Rate = ((Current_Amperes + Previous_Amperes)/2) * ((double)(Current_Time - Previous_Time)/3600000);
	C_D_Accumulated_mAH += BMS_Data.Pack_Charge_Discharge_Rate;

	if(Status_Flag.Pack_Charging == YES)
	{
		if(Previous_State == DISCHARGING)
		{
			C_D_Accumulated_mAH = 0.0;
			Previous_State = CHARGING;
		}
		else if (Previous_State == LOW_POWER_CONSUMPTION)
		{
			Previous_State = CHARGING;
		}

		BMS_Data.Pack_Capacity_Used -= BMS_Data.Pack_Charge_Discharge_Rate;
		if(BMS_Data.Pack_Capacity_Used < 0)
		{
			BMS_Data.Pack_Capacity_Used = 0.0;
		}
		else if(BMS_Data.Pack_Capacity_Used > BATTERY_CAPACITY)
		{
			BMS_Data.Pack_Capacity_Used = BATTERY_CAPACITY;
		}
	}
	else
	{
		if(Previous_State == CHARGING)
		{
			C_D_Accumulated_mAH = 0.0;
			Previous_State = DISCHARGING;
		}
		else if (Previous_State == LOW_POWER_CONSUMPTION)
		{
			Previous_State = DISCHARGING;
		}

		BMS_Data.Pack_Capacity_Used += BMS_Data.Pack_Charge_Discharge_Rate;
		if(BMS_Data.Pack_Capacity_Used > BATTERY_CAPACITY)
		{
			BMS_Data.Pack_Capacity_Used = BATTERY_CAPACITY;
		}
		else if(BMS_Data.Pack_Capacity_Used < 0)
		{
			BMS_Data.Pack_Capacity_Used = 0.0;
		}
	}
	Previous_Amperes = Current_Amperes;
	Previous_Time = Current_Time;

	BMS_Data.Pack_Capacity_Remaining = (float)(1 - (BMS_Data.Pack_Capacity_Used /(float)BATTERY_CAPACITY)) * 100;
}

/**
 * @brief  Function to read the pack voltage from BMS ASIC
 * @param  None
 * @retval None
 */
void BMS_Read_Pack_Voltage()
{
	uint16_t Pack_Voltage;
	uint8_t Address = PACK_VOLTAGE_ADDR;

	if(I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Address,1) == RESULT_OK)
	{
		if(I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,(uint8_t*)&Pack_Voltage,2) == RESULT_OK)
		{
			I2C_Error_Flag.I2C_Read_Pack_Volt_Flag = 0;
			BMS_Com_Restart = false;
		}
		else
		{
			I2C_Error_Flag.I2C_Read_Pack_Volt_Flag = 1;
			BMS_Com_Restart = true;
		}
	}
	else
	{
		I2C_Error_Flag.I2C_Read_Pack_Volt_Flag = 1;
		BMS_Com_Restart = true;
	}

	/* Hard coded formula defined by the ASIC manufacturer */
	BMS_Data.Pack_Voltage = ((uint16_t)(Pack_Voltage) * 1.8 * 32)/(4095);
	Pack_Data.values[6] = BMS_Data.Pack_Voltage;
}

/**
 * @brief  Function to sense the critical battery level
 * @param  None
 * @retval BATT_CRITICAL_LEVEL_REACHED: If battery voltage falls below set critical voltage level
 * 		   BATT_LEVEL_OK : If battery voltage is above critical voltage level
 */
uint8_t BMS_Check_Critical_Voltage()
{
	if(BMS_Data.Pack_Voltage <= CRITICAL_BATT_VOLTAGE)
	{
		return BATT_CRITICAL_LEVEL_REACHED;
	}
	else
	{
		return BATT_LEVEL_OK;
	}
}

/**
 * @brief  Function to read the pack current going into or out of pack from BMS ASIC
 * @param  None
 * @retval None
 */
void BMS_Read_Pack_Current()
{
	uint16_t Pack_Current;
	uint8_t Address = PACK_CURRENT_ADDR;

	if(I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1) == RESULT_OK)
	{
		if(I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*)&Pack_Current, 2) == RESULT_OK)
		{
			I2C_Error_Flag.I2C_Read_Pack_Current_Flag = 0;
			BMS_Com_Restart = false;
		}
		else
		{
			I2C_Error_Flag.I2C_Read_Pack_Current_Flag = 1;
			BMS_Com_Restart = true;
		}
	}
	else
	{
		I2C_Error_Flag.I2C_Read_Pack_Current_Flag = 1;
		BMS_Com_Restart = true;
	}

	/* Hard coded formula defined by ASIC manufacturer */
	BMS_Data.Pack_Current = (((((float)(Pack_Current) * 1.8) / (4095 * Current_Gain * SENSE_RESISTOR_VALUE))) * 1000);
	Pack_Data.values[7] = BMS_Data.Pack_Current;

	if(Current_Gain == CURRENT_GAIN_500X)
	{
		BMS_Data.Pack_Current_Adjusted = BMS_Data.Pack_Current +
				(BMS_Data.Pack_Current * SLOPE_500X) + CONSTANT_500X;
	}
	else if (Current_Gain == CURRENT_GAIN_50X)
	{
		BMS_Data.Pack_Current_Adjusted = BMS_Data.Pack_Current +
				(BMS_Data.Pack_Current * SLOPE_50X) + CONSTANT_50X;
	}
	else
	{
		BMS_Data.Pack_Current_Adjusted = BMS_Data.Pack_Current +
				(BMS_Data.Pack_Current * SLOPE_5X) + CONSTANT_5X;
	}

//	/* Convert adjusted pack current from amperes to the milli amperes */
//	BMS_Data.Pack_Current_Adjusted *= 1000;
//	uint8_t buffer[30];
//	uint8_t Len = sprintf(buffer,"%f\r",BMS_Data.Pack_Current_Adjusted);
//	BMS_Debug_COM_Write_Data(buffer,Len);
}

/**
 * @brief  Function to read the temperature of pack from BMS ASIC
 * @param  None
 * @retval None
 */
void BMS_Read_Pack_Temperature()
{
	uint16_t Pack_Data;
	float Lcl_Temperature_Volts = 0.0;

	uint8_t Address = PACK_TEMPERATURE_ADDR;

	if(I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Address,1) == RESULT_OK)
	{
		if(I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,(uint8_t*)&Pack_Data,2) == RESULT_OK)
		{
			I2C_Error_Flag.I2C_Read_Pack_Temp_Flag = 0;
			BMS_Com_Restart = false;
		}
		else
		{
			I2C_Error_Flag.I2C_Read_Pack_Temp_Flag = 1;
			BMS_Com_Restart = true;
		}
	}
	else
	{
		I2C_Error_Flag.I2C_Read_Pack_Temp_Flag = 1;
		BMS_Com_Restart = true;
	}

	/* Hard coded formula defined by ASIC manufacturer */
	Lcl_Temperature_Volts = ((float)(Pack_Data) * 1.8)/(4095);
	BMS_Data.Pack_Temperature_Degrees = (((Lcl_Temperature_Volts*1000)/(1.8527)) - 273.15);
}

/**
 * @brief  Function to round off the float variable between lower limit and upper limit
 * @param  None
 * @retval Value	: Constrained value
 */
float Constrain(float Value, float Lower_Limit, float Upper_Limit)
{
	if(Value > Upper_Limit)
	{
		Value = Upper_Limit;
	}
	else if(Value < Lower_Limit)
	{
		Value = Lower_Limit;
	}
	return Value;
}

/**
 * @brief  Function to return the charge discharge status of the BMS
 * @param  None
 * @retval CHARGING 	: BMS is in charging state
 * 		   DISCHARGING	: BMS is in discharging state
 * 		   LOW_POWER_CONSUMPTION: BMS is in IDLE mode without any power consumption
 */
uint8_t Get_BMS_Charge_Discharge_Status()
{
	return BMS_Data.Charging_Discharging_Status;
}

/**
 * @brief  Function to return the sleep mode status of the BMS
 * @param  None
 * @retval YES 	: BMS is sleep mode
 * 		   NO	: BMS is awake and querying the pack data
 */
uint8_t Get_BMS_Sleep_Mode_Status()
{
	return Status_Flag.BMS_In_Sleep;
}

/**
 * @brief  Function to return the individual cell voltages from the pack
 * @param  None
 * @retval Cell voltages in float
 */
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

/**
 * @brief  Function to return the pack voltage
 * @param  None
 * @retval Pack voltage in float
 */
float Get_BMS_Pack_Voltage()
{
	return BMS_Data.Pack_Voltage;
}

/**
 * @brief  Function to return pack capacity remaining
 * @param  None
 * @retval Battery capacity remaining in percentage
 */
float Get_BMS_Capacity_Remaining()
{
	return BMS_Data.Pack_Capacity_Remaining;
}

/**
 * @brief  Function to return pack capacity used
 * @param  None
 * @retval Battery capacity used in mAH
 */
float Get_BMS_Capacity_Used()
{
	return BMS_Data.Pack_Capacity_Used;
}

/**
 * @brief  Function to return the corrected pack current based on the equation derived in MATLAB
 * @param  None
 * @retval Pack current going into or out of the pack
 */
float Get_BMS_Pack_Current_Adj()
{
	return BMS_Data.Pack_Current_Adjusted;
}

/**
 * @brief  Function to calculate the pack voltage by adding all the cell voltages
 * @param  None
 * @retval Pack voltage
 */
float Get_BMS_Accumulated_Pack_Voltage()
{
	return (BMS_Data.Cell1_Voltage + BMS_Data.Cell2_Voltage + BMS_Data.Cell3_Voltage
			+ BMS_Data.Cell6_Voltage + BMS_Data.Cell7_Voltage + BMS_Data.Cell8_Voltage);
}

/**
 * @brief  Function to return the corrected pack current based on the equation derived in MATLAB
 * @param  None
 * @retval Pack current going into or out of the pack
 */
float Get_BMS_Pack_Current()
{
	/* Convert the ampere current to milli amperes by multiplying it by 1000 */
	return (BMS_Data.Pack_Current);
}

/**
 * @brief  Function to return the pack temperature
 * @param  None
 * @retval Pack temperature in degree celsius
 */
float Get_BMS_Pack_Temperature()
{
	return BMS_Data.Pack_Temperature_Degrees;
}

/**
 * @brief  Function to return total pack cycles used
 * @param  None
 * @retval Number of pack cyles used
 */
uint32_t Get_BMS_Total_Pack_Cycles()
{
	return BMS_Data.Pack_Total_Cycles;
}

/**
 * @brief  Function to return charge discharge rate of the pack
 * @param  None
 * @retval C_D rate
 */
float Get_BMS_Charge_Discharge_Rate()
{
	return BMS_Data.Pack_Charge_Discharge_Rate;
}

