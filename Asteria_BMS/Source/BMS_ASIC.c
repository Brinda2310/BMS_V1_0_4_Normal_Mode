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

double Current_Amperes = 0.0, Previous_Amperes = 0.0,Total_Pack_Capacity = 0.0;
uint32_t Current_Time = 0,Previous_Time = 0;

bool Last_Charge_Disharge_Status = LOW_POWER_CONSUMPTION;
uint32_t Error_Check_Data = 0;
bool Configuration_OK = false;

ISL_943203_Data BMS_Data;
BMS_Status_Flags Status_Flag;
I2C_Errors I2C_Error_Flag;
bool BMS_Com_Restart = false;
double C_D_Rate_Temp = 0;
uint16_t Current_Gain = CURRENT_GAIN_5X;

/* Function initializes the I2C communication between MCU and BMS IC */
void BMS_ASIC_Init()
{
	int8_t Max_Tries = 5;
	I2C_Error_Flag.I2C_Init_Flag = I2C_Init(BMS_I2C,I2C_OWN_ADDRESS,I2C_100KHZ,I2C_MASTER);
	while((I2C_Error_Flag.I2C_Init_Flag != RESULT_OK) && Max_Tries-- > 1)
	{
		BMS_Com_Restart = true;
	}
	if(Max_Tries < 1)
	{
		BMS_Debug_COM_Write_Data("Power restart is needed\r",24);
	}
}

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

/* Function to set the respective flags defined in the code read from BMS status registers */
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
	if(Flags & IS_INTERNAL_SCAN)
	{
		Status_Flag.Internal_Scan_Progress = NO;
	}
	else
	{
		Status_Flag.Internal_Scan_Progress = YES;
	}
}

/* Function to convert the cell voltages read from ISL registers(HEX) into float values */
static void Convert_To_Cell_Voltages(uint8_t *Data)
{
	unsigned short  *Integers;
	Integers = (unsigned short*)Data;

	/* Hard coded formulae defined by the ASIC manufacturer */
	BMS_Data.Cell1_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell2_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell3_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell4_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell5_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell6_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell7_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);
	BMS_Data.Cell8_Voltage = (*Integers++ * 1.8 * 8)/ (4095 * 3);

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

/*
 * Function to put ISL94203 into the sleep mode
 */
void BMS_Force_Sleep()
{
	int8_t Max_Tries = 5;
	I2C_Error_Flag.I2C_Force_Sleep = I2C_WriteData(BMS_I2C,BMS_ADDRESS,ISL_SLEEP_DATA,sizeof(ISL_SLEEP_DATA));
	while(I2C_Error_Flag.I2C_Force_Sleep != RESULT_OK && Max_Tries-- > 1)
	{
		BMS_Com_Restart = true;
	}
}

static void BMS_Set_Over_Voltage_Threshold(void)
{
	uint8_t Send_Data_Values[3],Index = 0;
	uint16_t Data_Value = 0x1E2A;
	uint16_t Pack_Data = 0;

	Send_Data_Values[Index++] = OV_THRESHOLD_ADDR;
	Send_Data_Values[Index++] = (Data_Value & 0xFF);
	Send_Data_Values[Index++] = ((Data_Value >> 8) & 0xFF);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,Send_Data_Values, Index);

	uint8_t Address = OV_THRESHOLD_ADDR;

	I2C_Error_Flag.I2C_Set_OV_Thresh_Flag = I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_Error_Flag.I2C_Set_OV_Thresh_Flag = I2C_ReadData(BMS_I2C,BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	if(Pack_Data == Data_Value || I2C_Error_Flag.I2C_Set_OV_Thresh_Flag == RESULT_OK)
	{
		Configuration_OK = true;
	}
}

static void BMS_Set_Over_Voltage_Recovery(void)
{
	uint8_t Send_Data_Values[3],Index = 0;
	uint16_t Data_Value = 0x0DD4;
	uint16_t Pack_Data = 0;

	Send_Data_Values[Index++] = OV_RECOVERY_ADDR;
	Send_Data_Values[Index++] = (Data_Value & 0xFF);
	Send_Data_Values[Index++] = ((Data_Value >> 8) & 0xFF);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS, Send_Data_Values, Index);

	uint8_t Address = OV_RECOVERY_ADDR;

	I2C_Error_Flag.I2C_Set_OV_Recovery_Flag = I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_Error_Flag.I2C_Set_OV_Recovery_Flag = I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	if (Pack_Data == Data_Value || I2C_Error_Flag.I2C_Set_OV_Recovery_Flag == RESULT_OK)
	{
		Configuration_OK = true;
	}
}

static void BMS_Set_Under_Voltage_Threshold(void)
{
	uint8_t Send_Data_Values[3],Index = 0;
	uint16_t Data_Value = 0x1BA9;
	uint16_t Pack_Data = 0;

	Send_Data_Values[Index++] = UV_THROSHOLD_ADDR;
	Send_Data_Values[Index++] = (Data_Value & 0xFF);
	Send_Data_Values[Index++] = ((Data_Value >> 8) & 0xFF);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS, Send_Data_Values, Index);

	uint8_t Address = UV_THROSHOLD_ADDR;

	I2C_Error_Flag.I2C_Set_UV_Thresh_Flag = I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_Error_Flag.I2C_Set_UV_Thresh_Flag = I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	if (Pack_Data == Data_Value || I2C_Error_Flag.I2C_Set_UV_Thresh_Flag == RESULT_OK)
	{
		Configuration_OK = true;
	}
}

static void BMS_Set_Under_Voltage_Recovery(void)
{
	uint8_t Send_Data_Values[3],Index = 0;
	uint16_t Data_Value = 0x0AAA;
	uint16_t Pack_Data = 0;

	Send_Data_Values[Index++] = UV_RECOVERY_ADDR;
	Send_Data_Values[Index++] = (Data_Value & 0xFF);
	Send_Data_Values[Index++] = ((Data_Value >> 8) & 0xFF);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS, Send_Data_Values, Index);

	uint8_t Address = UV_RECOVERY_ADDR;

	I2C_Error_Flag.I2C_Set_UV_Recovery_Flag = I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_Error_Flag.I2C_Set_UV_Recovery_Flag = I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	if (Pack_Data != 0x09FF || I2C_Error_Flag.I2C_Set_UV_Recovery_Flag == RESULT_OK)
	{
		Configuration_OK = true;
	}
}

static void BMS_Set_OV_LockOut_Threshold(void)
{
	uint8_t Send_Data_Values[3],Index = 0;
	uint16_t Data_Value = 0x0E7F;
	uint16_t Pack_Data = 0;

	Send_Data_Values[Index++] = OV_LOCKOUT_THRESHOLD_ADDR;
	Send_Data_Values[Index++] = (Data_Value & 0xFF);
	Send_Data_Values[Index++] = ((Data_Value >> 8) & 0xFF);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS, Send_Data_Values, Index);

	uint8_t Address = OV_LOCKOUT_THRESHOLD_ADDR;

	I2C_Error_Flag.I2C_Set_OV_Lockout_Thresh_Flag = I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_Error_Flag.I2C_Set_OV_Lockout_Thresh_Flag = I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	if (Pack_Data == Data_Value || I2C_Error_Flag.I2C_Set_OV_Lockout_Thresh_Flag == RESULT_OK)
	{
		Configuration_OK = true;
	}
}

static void BMS_Set_UV_LockOut_Threshold(void)
{
	uint8_t Send_Data_Values[3],Index = 0;
	uint16_t Data_Value = 0x0600;
	uint16_t Pack_Data = 0;

	Send_Data_Values[Index++] = UV_LOCKOUT_THRESHOLD_ADDR;
	Send_Data_Values[Index++] = (Data_Value & 0xFF);
	Send_Data_Values[Index++] = ((Data_Value >> 8) & 0xFF);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS, Send_Data_Values, Index);

	uint8_t Address = UV_LOCKOUT_THRESHOLD_ADDR;

	I2C_Error_Flag.I2C_Set_UV_Lockout_Thresh_Flag = I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_Error_Flag.I2C_Set_UV_Lockout_Thresh_Flag = I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	if (Pack_Data == Data_Value || I2C_Error_Flag.I2C_Set_UV_Lockout_Thresh_Flag == RESULT_OK)
	{
		Configuration_OK = true;
	}
}

static void BMS_Set_End_of_Charge_Threshold(void)
{
	uint8_t Send_Data_Values[3],Index = 0;
	uint16_t Data_Value = 0x0DFF;
	uint16_t Pack_Data = 0;

	Send_Data_Values[Index++] = EOC_THRESHOLD_ADDR;
	Send_Data_Values[Index++] = (Data_Value & 0xFF);
	Send_Data_Values[Index++] = ((Data_Value >> 8) & 0xFF);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS, Send_Data_Values, Index);

	uint8_t Address = EOC_THRESHOLD_ADDR;

	I2C_Error_Flag.I2C_Set_EOC_Thresh_Flag = I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_Error_Flag.I2C_Set_EOC_Thresh_Flag = I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	if (Pack_Data == Data_Value || I2C_Error_Flag.I2C_Set_EOC_Thresh_Flag == RESULT_OK)
	{
		Configuration_OK = true;
	}
}

//static uint8_t BMS_Set_OV_Delay_Timeout(void)
//{
//	uint8_t Result = 0xFF;
//
//	return Result;
//}
//
//static uint8_t BMS_Set_UV_Delay_Timeout(void)
//{
//	uint8_t Result = 0xFF;
//
//	return Result;
//}
//
//static uint8_t BMS_Set_Open_Wiring_Timeout(void)
//{
//	uint8_t Result = 0xFF;
//
//	return Result;
//}
//
static void BMS_Set_Internal_OT_Threshold(void)
{
	uint8_t Send_Data_Values[3],Index = 0;
	uint16_t Data_Value = 0x05A6;
	uint16_t Pack_Data = 0;

	Send_Data_Values[Index++] = INTERNAL_OT_THRESHOLD_ADDR;
	Send_Data_Values[Index++] = (Data_Value & 0xFF);
	Send_Data_Values[Index++] = ((Data_Value >> 8) & 0xFF);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS, Send_Data_Values, Index);

	uint8_t Address = INTERNAL_OT_THRESHOLD_ADDR;

	I2C_Error_Flag.I2C_Set_IOT_Thresh_Flag = I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_Error_Flag.I2C_Set_IOT_Thresh_Flag = I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2);

	if (Pack_Data == Data_Value || I2C_Error_Flag.I2C_Set_IOT_Thresh_Flag == RESULT_OK)
	{
		Configuration_OK = true;
	}
}

static void BMS_Set_Internal_OT_Recovery(void)
{
	uint8_t Send_Data_Values[3],Index = 0;
	uint16_t Data_Value = 0x0591;
	uint16_t Pack_Data = 0;

	Send_Data_Values[Index++] = INTERNAL_OT_RECOVERY_ADDR;
	Send_Data_Values[Index++] = (Data_Value & 0xFF);
	Send_Data_Values[Index++] = ((Data_Value >> 8) & 0xFF);

	I2C_WriteData(BMS_I2C, BMS_ADDRESS, Send_Data_Values, Index);

	uint8_t Address = INTERNAL_OT_RECOVERY_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);

	if (I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 2) == RESULT_OK)
	{
		if(Pack_Data == Data_Value)
		{
			Configuration_OK = true;
		}
	}
}

static void BMS_Disable_Cell_Balancing(void)
{
	uint8_t Send_Data_Values[3],Index = 0;
	uint16_t Pack_Data = 0;

	Send_Data_Values[Index++] = DISABLE_CELL_BALANCE_ADDR;
	Send_Data_Values[Index++] = 0x00;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS, Send_Data_Values, Index);

	uint8_t Address = DISABLE_CELL_BALANCE_ADDR;

	I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);

	if (I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*) &Pack_Data, 1) == RESULT_OK)
	{
		if(Pack_Data == 0x00)
		{
			Configuration_OK = true;
		}
	}
}

void BMS_Configure_Parameters(void)
{
	BMS_Set_Over_Voltage_Threshold();
	BMS_Set_Over_Voltage_Recovery();
	BMS_Set_Under_Voltage_Threshold();
	BMS_Set_Under_Voltage_Recovery();
	BMS_Set_OV_LockOut_Threshold();
	BMS_Set_UV_LockOut_Threshold();
	BMS_Set_End_of_Charge_Threshold();
	BMS_Set_Internal_OT_Threshold();
	BMS_Disable_Cell_Balancing();
	BMS_Set_Internal_OT_Recovery();

	if(Configuration_OK == false)
	{
		BMS_Debug_COM_Write_Data("Configuration Failed\r",21);
	}
}

/* Function to set the gain value to the EEPROM of BMS ASIC */
uint8_t BMS_Set_Current_Gain(uint16_t Gain_Setting)
{
	uint8_t Gain_Value, Result;
	uint8_t Register_Address = 0, Send_Data_Values[2];
	int8_t Max_Tries = 5;
	Current_Gain = Gain_Setting;

	/* Before writing any value,make sure that other settings in the 0x85 register are not disturbed
	 * So read the value first and then just change the 4th and 5th bit value in the register */
	Register_Address = CURRENT_GAIN_SETTING_ADDR;
	I2C_Error_Flag.I2C_Set_Current_Gain_Flag = I2C_WriteData(BMS_I2C, BMS_ADDRESS, &Register_Address, 1);
	I2C_Error_Flag.I2C_Set_Current_Gain_Flag = I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, &Gain_Value, 1);

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
		if (I2C_WriteData(BMS_I2C, BMS_ADDRESS, Send_Data_Values,sizeof(Send_Data_Values)) == RESULT_OK)
		{
			Result = WRITE_OK;
			I2C_Error_Flag.I2C_Set_Current_Gain_Flag = RESULT_OK;
		}
		else
		{
			Result = WRITE_ERROR;
			I2C_Error_Flag.I2C_Set_Current_Gain_Flag = RESULT_ERROR;
		}
	}
	while(I2C_Error_Flag.I2C_Set_Current_Gain_Flag != RESULT_OK && Max_Tries-- > 1)
	{
		BMS_Com_Restart = true;;
	}
	return Result;
}

/* Function to set the maximum of charge/discharge pack cycles */
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

/* This function to query the status flags of various RAM registers */
void BMS_Read_RAM_Status_Register()
{
	int8_t Max_Tries = 5;
	typedef union
	{
		uint8_t Data[4];
		uint32_t Stat_Flags;
	}Bytes_to_integer;

	Bytes_to_integer RAM_Flags;

	uint8_t Register_Address = RAM_STATUS_REG_ADDR;
	I2C_Error_Flag.I2C_Read_Status_Flag = I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Register_Address,1);
	I2C_Error_Flag.I2C_Read_Status_Flag = I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,RAM_Flags.Data,4);
	Error_Check_Data = RAM_Flags.Stat_Flags;
	BMS_Set_Status_Flags(RAM_Flags.Stat_Flags);

	while(I2C_Error_Flag.I2C_Read_Status_Flag != RESULT_OK && Max_Tries-- > 1)
	{
		BMS_Com_Restart = true;;
	}
}

/* Function to read individual cell voltages inside the pack */
void BMS_Read_Cell_Voltages()
{
	int8_t Max_Tries = 5;
	uint8_t Cell_Voltages[CELL_VOLTAGES_DATA_SIZE];

	uint8_t Register_Address = CELL_VOLTAGE_ADDR;
	I2C_Error_Flag.I2C_Read_Cells_Flag = I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Register_Address,1);
	/* Sequential read method of ISL is used to read all the cell voltages as they are in sequence in EEPROM */
	I2C_Error_Flag.I2C_Read_Cells_Flag = I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,Cell_Voltages,CELL_VOLTAGES_DATA_SIZE);
	/* This function converts the read HEX values from ISL; convert them to integer and then does calculation
	 * to find the actual cell voltage */
	Convert_To_Cell_Voltages(Cell_Voltages);

	while(I2C_Error_Flag.I2C_Read_Cells_Flag != RESULT_OK && Max_Tries-- > 1)
	{
		BMS_Com_Restart = true;;
	}
}

/* Function to calculate the initial battery capacity */
void BMS_Estimate_Initial_Capacity(void)
{
	float Batt_Volt_Per_Cell = 0, volt_z = 0, pow_volt_z = 0, Battery_Estimate = 0;

	Batt_Volt_Per_Cell = Get_BMS_Pack_Voltage() / (float) BATTERY_CELLS_COUNT;

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

//	uint8_t Buffer[50];
//	uint8_t Length = sprintf(Buffer,"Batt Rem = %0.3f%c\r",BMS_Data.Pack_Capacity_Remaining,'%');
//	Length += sprintf(&Buffer[Length],"Batt Used = %0.3fmAH\r",BMS_Data.Pack_Capacity_Used);
//	BMS_Debug_COM_Write_Data(Buffer,Length);
}

/* Function to calculate the pack capacity used over the time(dt = 30ms) */
void BMS_Estimate_Capacity_Used()
{
	Current_Time = Get_System_Time_Millis();
	Current_Amperes = Get_BMS_Pack_Current();
	BMS_Data.Pack_Charge_Discharge_Rate = ((Current_Amperes + Previous_Amperes)/2) * ((double)(Current_Time - Previous_Time)/3600000);
	C_D_Rate_Temp +=BMS_Data.Pack_Charge_Discharge_Rate;

	if(Status_Flag.Pack_Charging == YES)
	{
		BMS_Data.Pack_Capacity_Used -= BMS_Data.Pack_Charge_Discharge_Rate;
		if(BMS_Data.Pack_Capacity_Used < 0)
		{
			BMS_Data.Pack_Capacity_Used = 0.0;
		}
		else if(BMS_Data.Pack_Capacity_Used > 100)
		{
			BMS_Data.Pack_Capacity_Used = 100.0;
		}
	}
	else
	{
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

/* Function to read the pack voltage from ISL IC */
void BMS_Read_Pack_Voltage()
{
	uint16_t Pack_Data;
	uint8_t Address = PACK_VOLTAGE_ADDR;
	int8_t Max_Tries = 5;

	I2C_Error_Flag.I2C_Read_Pack_Volt_Flag = I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Address,1);
	I2C_Error_Flag.I2C_Read_Pack_Volt_Flag = I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,(uint8_t*)&Pack_Data,2);

	/* Hard coded formula defined by the ASIC manufacturer */
	BMS_Data.Pack_Voltage = ((uint16_t)(Pack_Data) * 1.8 * 32)/(4095);

	while(I2C_Error_Flag.I2C_Read_Pack_Volt_Flag != RESULT_OK && Max_Tries-- > 1)
	{
		BMS_Com_Restart = true;;
	}
}

/* Function to read the pack current going into or out of pack from ISL IC */
void BMS_Read_Pack_Current()
{
	uint16_t Pack_Data;
	uint8_t Address = PACK_CURRENT_ADDR;
	int8_t Max_Tries = 5;

	I2C_Error_Flag.I2C_Read_Pack_Current_Flag = I2C_WriteData(BMS_I2C, BMS_ADDRESS,&Address, 1);
	I2C_Error_Flag.I2C_Read_Pack_Current_Flag = I2C_ReadData(BMS_I2C, BMS_ADDRESS | 0x01, (uint8_t*)&Pack_Data, 2);

	/* Hard coded formula defined by ASIC manufacturer */
	BMS_Data.Pack_Current = (((float)(Pack_Data) * 1.8) / (4095 * Current_Gain * SENSE_RESISTOR_VALUE));

	while(I2C_Error_Flag.I2C_Read_Pack_Current_Flag != RESULT_OK && Max_Tries-- > 1)
	{
		BMS_Com_Restart = true;;
	}
}

/* Function to read the temperature of pack from BMS ASIC */
void BMS_Read_Pack_Temperature()
{
	uint16_t Pack_Data;
	float Lcl_Temperature_Volts = 0.0;
	int8_t Max_Tries = 5;

	uint8_t Address = PACK_TEMPERATURE_ADDR;

	I2C_Error_Flag.I2C_Read_Pack_Temp_Flag = I2C_WriteData(BMS_I2C,BMS_ADDRESS,&Address,1);
	I2C_Error_Flag.I2C_Read_Pack_Temp_Flag = I2C_ReadData(BMS_I2C,BMS_ADDRESS|0x01,(uint8_t*)&Pack_Data,2);

	/* Hard coded formula defined by ASIC manufacturer */
	Lcl_Temperature_Volts = ((float)(Pack_Data) * 1.8)/(4095);
	BMS_Data.Pack_Temperature_Degress = (((Lcl_Temperature_Volts*1000)/(1.8527)) - 273.15);

	while(I2C_Error_Flag.I2C_Read_Pack_Temp_Flag != RESULT_OK && Max_Tries-- > 1)
	{
		BMS_Com_Restart = true;;
	}
}

/* Function to round off the float variable between lower limit and upper limit */
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

/* All these function just return the values that are updated from BMS ASIC registers */
uint8_t Get_BMS_Charge_Discharge_Status()
{
	return BMS_Data.Charging_Discharging_Status;
}

uint8_t Get_BMS_Sleep_Mode_Status()
{
	return Status_Flag.BMS_In_Sleep;
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

float Get_BMS_Capacity_Remaining()
{
	return BMS_Data.Pack_Capacity_Remaining;
}

float Get_BMS_Capacity_Used()
{
	return BMS_Data.Pack_Capacity_Used;
}

float Get_BMS_Pack_Current_Adj()
{
	return BMS_Data.Pack_Current_Adjusted;
}

float Get_BMS_Accumulated_Pack_Voltage()
{
	return (BMS_Data.Cell1_Voltage + BMS_Data.Cell2_Voltage + BMS_Data.Cell3_Voltage
			+ BMS_Data.Cell6_Voltage + BMS_Data.Cell7_Voltage + BMS_Data.Cell8_Voltage);
}

float Get_BMS_Pack_Current()
{
	float Temp_Current = 0.0;
	/* Convert the ampere current to milli amperes by multiplying it by 1000 */

	Temp_Current = (BMS_Data.Pack_Current * 1000);

	if(Current_Gain == CURRENT_GAIN_500X)
	{
		BMS_Data.Pack_Current_Adjusted = Temp_Current +
				(Temp_Current * SLOPE_500X) + CONSTANT_500X;
	}
	else if (Current_Gain == CURRENT_GAIN_50X)
	{
		BMS_Data.Pack_Current_Adjusted = Temp_Current +
				(Temp_Current * SLOPE_50X) + CONSTANT_50X;
	}
	else
	{
		BMS_Data.Pack_Current_Adjusted = Temp_Current +
				(Temp_Current * SLOPE_5X) + CONSTANT_5X;
	}

	return Temp_Current;
}

float Get_BMS_Pack_Temperature()
{
	return BMS_Data.Pack_Temperature_Degress;
}
float Get_BMS_Total_Pack_Cycles()
{
	return BMS_Data.Pack_Total_Cycles;
}
float Get_BMS_Charge_Discharge_Rate()
{
	return BMS_Data.Pack_Charge_Discharge_Rate;
}

