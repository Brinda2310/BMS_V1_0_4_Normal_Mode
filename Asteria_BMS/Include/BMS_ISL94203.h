/*
 * BMS_ISL94203.h
 *
 *  Created on: 20-Jan-2017
 *      Author: NIKHIL
 */

#ifndef BMS_ISL94203_H_
#define BMS_ISL94203_H_

#include "I2C_API.h"

#define ISL94203_ADDRESS									0x50
#define I2C_OWN_ADDRESS										0x0F
#define EEPROM_PAGE_SIZE									4
#define EEPROM_WRITE_DELAY									30
#define USER_EEPROM_START_ADDRESS							0x50
#define I2C_100KHZ											0x90942027
#define ISL_I2C												I2C_1

typedef enum
{
	WRITE_OK = 0,WRITE_ERROR
}ISL_WriteStatus;

void ISL94203_Init();
void ISL94203_EEPROM_Access_Enable();
void ISL94203_RAM_Access_Enable();
void Access_RAM_Data();
void ISL94203_Force_Sleep();
void ISL94203_Read_Status_Register(uint8_t *Data);
ISL_WriteStatus ISL94203_User_EEPROM_Write(uint8_t Memory_Address,uint8_t *Data_Ptr,uint8_t Data_Size);
void ISL94203_User_EEPROM_Read(uint8_t Memory_Address,uint8_t *Buffer,uint8_t Data_Size);

#endif /* BMS_ISL94203_H_ */
