/*
 * BMS_Flash.c
 *
 *  Created on: 14-May-2017
 *      Author: NIKHIL
 */

#include <BMS_Flash.h>

/* Variable to store the memory location to which BMS data is written inside the internal flash */
uint32_t Address_Write_Location = ADDR_FLASH_PAGE_80;
uint32_t Previous_Data_Write_Location = 0;

/* Logic to write to the flash is implemented in such a way that at power up, function will read all the
 * previously written address from flash's address book location. If flash's address book location returns
 * the location which is out of bounds then BMS data is written to start address of flash. This variables
 * tells that the data is written to the very first location of BMS data address */
bool Power_Up_Flag = true;

/* This flag becomes false when valid address for writing BMS data is found in flash's another location */
static bool Continue_Read_Flash = true;

/* Holds the current read address from flash's */
uint64_t Read_Address_Location = 0;

/**
 * @brief  Function to write the user bytes to the EEPROM section of BMS IC
 * @param  Memory_Address	: Memory address to which BMS data is to be written to flash
 * @param  TxBuffer			: Pointer to the data buffer which is to be written to flash
 * @param  Size 			: Number of bytes to be written to flash
 * @retval MEM_DATA_SIZE_OVERLFOW		: If data size is greater than 2K
 * 		   WRITE_MEM_DATA_SIZE_INVALID	: If data size if not multiple of 8 bytes
 * 		   MEM_WRITE_OK					: Memory write is successful
 */
uint8_t Write_Data(uint32_t *Memory_Address,uint8_t *TxBuffer,uint8_t Size)
{
	if ((*Memory_Address + Size) > ADDR_FLASH_PAGE_71)
	{
		return MEM_DATA_SIZE_OVERLFOW;
	}

	while(Continue_Read_Flash == true)
	{
		MCU_Flash_Read(Address_Write_Location,(Address_Write_Location+READ_SIZE),(uint32_t*)&Read_Address_Location);

		if((uint32_t)Read_Address_Location != *Memory_Address &&
				((uint32_t)Read_Address_Location >= ADDR_FLASH_PAGE_71) &&  ((uint32_t)Read_Address_Location <= ADDR_FLASH_PAGE_70))
		{
			if(Size % WRITE_SIZE == 0)
			{
				MCU_Flash_Write(*Memory_Address,((*Memory_Address) + Size),(uint64_t*)TxBuffer,ERASE);
				MCU_Flash_Write(Address_Write_Location,(Address_Write_Location + WRITE_SIZE),
						(uint64_t*)Memory_Address,ERASE);
				*Memory_Address += Size;
				Address_Write_Location += WRITE_SIZE;
				Continue_Read_Flash = false;
				Power_Up_Flag = false;
			}
			else
			{
				return MEM_DATA_SIZE_INVALID;
			}
		}
		else if ((uint32_t)Read_Address_Location == *Memory_Address)
		{
			Previous_Data_Write_Location = *Memory_Address;
			*Memory_Address += Size;
			Address_Write_Location += WRITE_SIZE;
		}
		else
		{
			Continue_Read_Flash = false;
		}
	}

	if(Continue_Read_Flash == false)
	{
		uint8_t Erase_Operation;
		if(Power_Up_Flag == true)
		{
			if((*Memory_Address) < ADDR_FLASH_PAGE_71)
			{
				Erase_Operation = DO_NOT_ERASE;
			}
			else
			{
				Erase_Operation = ERASE;
				*Memory_Address = ADDR_FLASH_PAGE_70;
				Address_Write_Location = ADDR_FLASH_PAGE_80;
			}

			MCU_Flash_Write(*Memory_Address, ((*Memory_Address) + Size),(uint64_t*) TxBuffer, Erase_Operation);
			MCU_Flash_Write(Address_Write_Location,(Address_Write_Location + WRITE_SIZE),
					(uint64_t*)Memory_Address,Erase_Operation);
			*Memory_Address += Size;
			Address_Write_Location += WRITE_SIZE;
		}
		Power_Up_Flag = true;
	}
	return MEM_WRITE_OK;

}

