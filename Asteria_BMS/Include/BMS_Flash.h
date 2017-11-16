/*
 * BMS_Flash.h
 *
 *  Created on: 14-May-2017
 *      Author: NIKHIL
 */

#ifndef BMS_FLASH_H_
#define BMS_FLASH_H_

#include "FLASH_API.h"

#define SECTOR_SIZE 				2048 		/* STM32L4 flash's sector size is 2K */
#define READ_SIZE					8			/* Number of bytes to be read/written to the flash */
#define WRITE_SIZE					READ_SIZE

/* Flash's memory location related enumerations
 * @MEM_DATA_SIZE_OVERLFOW	: Size of data buffer given for writing is more than flash's page size
 * @MEM_DATA_SIZE_INVALID	: Size of data given for writing is not multiple of 8Bytes
 * @MEM_WRITE_OK			: Writing to flash locations is successful
 * @MEM_READ_OK				: Reading from flash memory locations is successful */
enum Mem_Write_Results
{
	MEM_DATA_SIZE_OVERLFOW = 0,MEM_DATA_SIZE_INVALID,MEM_WRITE_OK,MEM_READ_OK
};

/* Function prototypes */
uint8_t Write_Data(uint32_t *Memory_Address,uint8_t *TxBuffer,uint8_t Size);

#endif /* BMS_FLASH_H_ */
