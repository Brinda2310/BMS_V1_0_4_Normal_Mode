/*
 * BMS_Flash.h
 *
 *  Created on: 14-May-2017
 *      Author: NIKHIL
 */

#ifndef BMS_FLASH_H_
#define BMS_FLASH_H_

#include "FLASH_API.h"

/* STM32L4 flash's sector size is 2K */
#define SECTOR_SIZE 				2048
/* Number of bytes to be read/written to the flash */
#define READ_SIZE					8
#define WRITE_SIZE					READ_SIZE

enum Mem_Write_Results
{
	MEM_ADDRESS_INVALID = 0, MEM_DATA_SIZE_OVERLFOW,MEM_DATA_SIZE_INVALID,MEM_WRITE_OK,MEM_READ_OK
};

/* Function prototypes */
uint8_t Write_Data(uint32_t *Memory_Address,uint8_t *TxBuffer,uint8_t Size);

#endif /* BMS_FLASH_H_ */
