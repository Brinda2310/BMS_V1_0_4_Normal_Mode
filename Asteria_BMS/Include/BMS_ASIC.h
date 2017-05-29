/*
 * BMS_ASIC.h
 *
 *  Created on: 20-Jan-2017
 *      Author: NIKHIL
 */

#ifndef BMS_ASIC_H_
#define BMS_ASIC_H_

#include "I2C_API.h"

#define BMS_ADDRESS									0x50	/* ISL94203 slave address */
#define BMS_I2C										I2C_1	/* I2C bus of MCU to be used for ISL */
#define I2C_OWN_ADDRESS								0x0F	/* This is the MCU's own address */
#define EEPROM_PAGE_SIZE							4		/* Max size of EEPROM page in ISL */
#define EEPROM_WRITE_DELAY							30		/* Time gap between two successiveEEPROM writes */

/*
 * Coefficients required for calculating used mAh from the no-load battery voltage
 * y = 0.051156*z^5 -0.69723*z^4 + 1.9663*z^3 -3.0335^z2 + 14.781*z +79.795
 * where z = (x - 3.94) / 0.13453
 * where x is battery voltage;
 * y is estimated battery capacity remaining (in mAh)
 */
#define BATT_EST_COEFF_0 			79.795				//z^0
#define BATT_EST_COEFF_1 			14.781				//z^1
#define BATT_EST_COEFF_2 			-3.0335				//z^2
#define BATT_EST_COEFF_3 			1.9663				//z^3
#define BATT_EST_COEFF_4 			-0.69723			//z^4
#define BATT_EST_COEFF_5 			0.051156			//z^5
#define BATT_EST_Mu					3.94				//constant used 3.94 (mean value)in above equation
#define BATT_EST_Sigma 				0.13453				//constant used 0.13453 (standard deviation)
#define BATTERY_CAPACITY			11000
#define BATTERY_CELLS_COUNT			6
/* BMS ISL94203 Internal register's addresses */
#define USER_EEPROM_START_ADDRESS					0x50	/* ISL's user EEPROM start address */
#define RAM_STATUS_REG_ADDRESS						0x80	/* ISL's RAM status flags address */
#define PACK_CURRENT_ADDR							0x8E	/* ISL's pack current register address */
#define CELL_VOLTAGE_ADDR							0x90	/* ISL's cell voltages register address */
#define PACK_TEMPERATURE_ADDR						0xA0	/* ISL's internal temperature register address */
#define PACK_VOLTAGE_ADDR							0xA6	/* ISL's pack voltage register address */

/* Here all the flags are defined which are required for Asteria BMS */
/* RAM location 0x80 status flags */
#define IS_ANY_CELL_V_OVER_THRESHOLD				(1 << 0)
#define IS_ANY_CELL_V_UNDER_THRESHOLD				(1 << 2)
#define IS_DISCHARGE_OVER_TEMP						(1 << 4)
#define IS_DISCHARGE_UNDER_TEMP						(1 << 5)
#define IS_CHARGE_OVER_TEMP							(1 << 6)
#define IS_CHARGE_UNDER_TEMP						(1 << 7)

/* RAM location 0x81 status flags */
#define IS_INTERNAL_OVER_TEMP						(1 << 8)
#define IS_CHARGE_OVER_CURRENT						(1 << 9)
#define IS_DISCHARGE_OVER_CURRENT					(1 << 10)
#define IS_DISHARGE_SHORT_CIRCUIT					(1 << 11)
#define IS_CELL_FAIL								(1 << 12)
#define IS_OPEN_WIRE								(1 << 13)
#define IS_END_OF_CHARGE							(1 << 15)

/* RAM location 0x82 status flags */
#define IS_PACK_CHARGING							(1 << 18)
#define IS_PACK_DISCHARGING							(1 << 19)
#define IS_INTERNAL_SCAN							(1 << 22)

/* RAM location 0x83 status flags */
#define IS_CELL_BALANCE_OVER_T						(1 << 24)
#define IS_CELL_BALANCE_UNDER_T						(1 << 25)
#define IS_CELL_BALANCE_OVER_V						(1 << 26)
#define IS_CELL_BALANCE_UNDER_V						(1 << 27)
#define IS_ISL_IN_IDLE								(1 << 28)
#define IS_ISL_IN_DOZE								(1 << 29)
#define IS_ISL_IN_SLEEP								(1 << 30)

#define NUMBER_OF_CELLS								8		/* The number of cells the battery has */
#define CELL_VOLTAGES_DATA_SIZE						(2*NUMBER_OF_CELLS)
#define SENSE_RESISTOR_VALUE						1e-3	/* Current sense resistor value used in hardware */
#define CURRENT_GAIN								5		/* Set the current gain as per sense resistor value */
#define MINIMUM_CURRENT_CONSUMPTION					50		/* If current consumption is less than 50mA
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	  for specific time then put the BMS to sleep */
#define MAXIMUM_PACK_VOLTAGE						25		/* Maximum value of pack voltage */

/* Enums to define the write results;
 * @WRITE_OK	: Write operation is successful
 * @WRITE_ERROR	: Write operation is unsuccessful */
enum Write_Result
{
	WRITE_OK = 0,WRITE_ERROR
};

/* Enums to define the current battery status
 * @DISCHARGING 		 : Current is going out of pack
 * @CHARGING			 : Current is coming into the pack
 * @LOW_POWER_CONSUMPTION: BMS is in IDLE state */
enum Pack_Status
{
	DISCHARGING = 0,CHARGING,LOW_POWER_CONSUMPTION
};

/* Enums to define the flags inside the BMS IC registers */
enum Flag_Results
{
	NO = 0, YES
};

/* Structure holding all the flags which are queried from BMS IC and same will be updated to use in code
 * after reading them from registers */
typedef struct
{
	uint8_t Cell_Over_Voltage		:1;
	uint8_t Over_Voltage_Lockout  	:1;
	uint8_t Cell_Under_Voltage		:1;
	uint8_t Under_Voltage_Lockout 	:1;
	uint8_t Discharge_Over_Temp	  	:1;
	uint8_t Discharge_Under_Temp  	:1;
	uint8_t Charge_Over_Temp 	  	:1;
	uint8_t Charge_Under_Temp 	  	:1;

	uint8_t Internal_Over_Temp		:1;
	uint8_t Charge_Overcurrent  	:1;
	uint8_t Discharge_Overcurrent	:1;
	uint8_t Charge_Shortcircuit 	:1;
	uint8_t Cell_Fail			  	:1;
	uint8_t Open_Wire			  	:1;
	uint8_t Res1			 	  	:1;
	uint8_t End_Of_Charge	 	  	:1;

	uint8_t Load_Present			:1;
	uint8_t Charger_Present		  	:1;
	uint8_t Pack_Charging			:1;
	uint8_t Pack_Discharging	 	:1;
	uint8_t Res2					:1;
	uint8_t Res3					:1;
	uint8_t Internal_Scan_Progress 	:1;
	uint8_t Low_Voltage_Charge	  	:1;

	uint8_t Cell_Balance_OT			:1;
	uint8_t Cell_Balance_UT		  	:1;
	uint8_t Cell_Balance_OV			:1;
	uint8_t Cell_Balance_UV		 	:1;
	uint8_t BMS_In_Idle				:1;
	uint8_t BMS_In_Doze				:1;
	uint8_t BMS_In_Sleep		 	:1;
	uint8_t Res4				  	:1;

}BMS_Status_Flags;

/* Variable which holds the value of all the status flags updated from ISL at 25Hz */
extern BMS_Status_Flags Status_Flag;
extern float Pack_Capacity;

/* Structure holding all the variables to be logged on SD card and to be used in the code */
typedef struct
{
	float Cell1_Voltage;
	float Cell2_Voltage;
	float Cell3_Voltage;
	float Cell4_Voltage;
	float Cell5_Voltage;
	float Cell6_Voltage;
	float Cell7_Voltage;
	float Cell8_Voltage;

	float Pack_Voltage;
	float Pack_Current;
	float Pack_Temperature_Degress;

	float Pack_Capacity_Remaining;
	float Pack_Capacity_Used;

	uint16_t Pack_Cycles;
	uint8_t Charging_Discharging_Status;

}ISL_943203_Data;

/* Function prototypes defined in the BMS_ASIC.c file */
void BMS_ASIC_Init();
uint8_t BMS_User_EEPROM_Write(uint8_t Memory_Address,uint8_t *Data_Ptr,uint8_t Data_Size);
void BMS_User_EEPROM_Read(uint8_t Memory_Address,uint8_t *Buffer,uint8_t Data_Size);
void BMS_Force_Sleep();
void BMS_Read_RAM_Status_Register(void);
void BMS_Read_Cell_Voltages(void);
void BMS_Read_Pack_Voltage(void);
void BMS_Read_Pack_Current(void);
void BMS_Read_Pack_Temperature(void);

void BMS_Estimate_Initial_Capacity(void);
void BMS_Estimate_Capacity_Used(void);
float Constrain(float var, float llimit, float ulimit);

uint8_t Get_BMS_Charge_Discharge_Status(void);
float Get_BMS_Initial_Capacity(void);
float Get_BMS_Capacity_Used(void);
float Get_Cell1_Voltage(void);
float Get_Cell2_Voltage(void);
float Get_Cell3_Voltage(void);
float Get_Cell6_Voltage(void);
float Get_Cell7_Voltage(void);
float Get_Cell8_Voltage(void);
float Get_BMS_Pack_Voltage(void);
float Get_BMS_Pack_Current(void);
float Get_BMS_Pack_Temperature(void);

#endif /* BMS_BMS_H_ */
