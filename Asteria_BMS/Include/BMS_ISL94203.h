/*
 * BMS_ISL94203.h
 *
 *  Created on: 20-Jan-2017
 *      Author: NIKHIL
 */

#ifndef BMS_ISL94203_H_
#define BMS_ISL94203_H_

#include "I2C_API.h"

#define BMS_ADDRESS									0x50
#define I2C_OWN_ADDRESS										0x0F
#define EEPROM_PAGE_SIZE									4
#define EEPROM_WRITE_DELAY									30
#define USER_EEPROM_START_ADDRESS							0x50
#define I2C_100KHZ											0x90942027
#define BMS_I2C												I2C_1

#define RAM_0x80_STATUS											0x80
#define RAM_0x81_STATUS											0x81
#define RAM_0x82_STATUS											0x82
#define RAM_0x83_STATUS											0x83

#define PACK_CURRENT									0x8E
#define CELL1_VOLTAGE									0x90
#define CELL2_VOLTAGE									0x92
#define CELL3_VOLTAGE									0x94
#define CELL4_VOLTAGE									0x96
#define CELL5_VOLTAGE									0x98
#define CELL6_VOLTAGE									0x9A
#define CELL7_VOLTAGE									0x9C
#define CELL8_VOLTAGE									0x9E

#define BMS_PACK_TEMPERATURE							0xA0
#define PACK_VOLTAGE									0xA6
/* RAM location 0x80 status flags */
#define IS_CHARGE_UNDER_TEMP									(1 << 7)
#define IS_CHARGE_OVER_TEMP										(1 << 6)
#define IS_DISCHARGE_UNDER_TEMP									(1 << 5)
#define IS_DISCHARGE_OVER_TEMP									(1 << 4)
#define IS_ANY_CELL_V_UNDER_THRESHOLD							(1 << 2)
#define IS_ANY_CELL_V_OVER_THRESHOLD							(1 << 0)

/* RAM location 0x81 status flags */
#define IS_END_OF_CHARGE										(1 << 7)
#define IS_OPEN_WIRE											(1 << 5)
#define IS_CELL_FAIL											(1 << 4)
#define IS_DISHARGE_SHORT_CIRCUIT								(1 << 3)
#define IS_DISCHARGE_OVER_CURRENT								(1 << 2)
#define IS_CHARGE_OVER_CURRENT									(1 << 1)
#define IS_INTERNAL_OVER_TEMP									(1 << 0)

/* RAM location 0x82 status flags */
#define IS_INTERNAL_SCAN										(1 << 6)
#define IS_PACK_DISCHARGING										(1 << 3)
#define IS_PACK_CHARGING										(1 << 2)
//#define IS_CHARGER_PRESENT									(1 << 1)
//#define IS_LOAD_PRESENT										(1 << 0)

/* RAM location 0x83 status flags */
#define IS_ISL_IN_SLEEP											(1 << 6)
#define IS_ISL_IN_DOZE											(1 << 5)
#define IS_ISL_IN_IDLE											(1 << 4)
#define IS_CELL_BALANCE_UNDER_V									(1 << 3)
#define IS_CELL_BALANCE_OVER_V									(1 << 2)
#define IS_CELL_BALANCE_UNDER_T									(1 << 1)
#define IS_CELL_BALANCE_OVER_T									(1 << 0)

#define SENSE_RESISTOR_VALUE									1
#define CURRENT_GAIN											5
typedef struct
{
	uint8_t Charge_Under_Temp:1;
	uint8_t Charge_Over_Temp:1;
	uint8_t Discharge_Under_Temp:1;
	uint8_t Discharge_Over_Temp:1;
}Error_Status_Flags;

typedef enum
{
	WRITE_OK = 0,WRITE_ERROR
}ISL_WriteStatus;

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
	float Pack_Temperature;
}ISL_943203_Data;

ISL_943203_Data BMS_Data;
void BMS_Init();
void BMS_EEPROM_Access_Enable();
void BMS_RAM_Access_Enable();
ISL_WriteStatus BMS_User_EEPROM_Write(uint8_t Memory_Address,uint8_t *Data_Ptr,uint8_t Data_Size);
void BMS_User_EEPROM_Read(uint8_t Memory_Address,uint8_t *Buffer,uint8_t Data_Size);
void BMS_Force_Sleep();
void BMS_RAM_Status_Register(uint8_t Register_Address,uint8_t *Data);
void BMS_Read_Cell_Voltages(uint8_t Register_Address,uint8_t *Data);
void BMS_Read_Pack_Data(uint8_t Register_Address);
void BMS_Read_Pack_Temperature();
void Convert_Bytes_To_Short(uint8_t *Data,uint16_t *Short_Data);

#endif /* BMS_BMS_H_ */
