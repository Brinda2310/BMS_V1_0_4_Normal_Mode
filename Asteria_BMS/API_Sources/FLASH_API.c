/******************************************************************************
  * @file    FLASH_API.c
  * @author  NIKHIL INGALE
  * @date    20-Jan-2017
  * @brief   This file contains all the functions definations for the FLASH
  *          module driver
  ******************************************************************************/
#include "FLASH_API.h"

static uint32_t Get_Flash_Page_Number(uint32_t Address);

/**
 * @brief  Write data to the flash of the micro controller
 * @param  User_Flash_Start_Address		: Start address of flash from where data is to be written
 * @param  User_Flash_End_Address		: End address of flash upto which data is to be written
 * @param  TxBuffer						: Pointer to the data buffer to be written to flash
 * @retval RES_ERROR					: Not Successful
 * 		   RES_OK						: Successful
 */
uint8_t MCU_Flash_Write(uint32_t User_Flash_Start_Address,uint32_t User_Flash_End_Address,uint64_t *TxBuffer)
{
	uint32_t FirstPage = 0, NoOfPages = 0;
	uint32_t PageError = 0,Index = 0;
	bool EraseSuccessful = false;
	uint8_t Result = RESULT_OK;
	uint32_t Address;

#ifdef BMS_VERSION
	static FLASH_EraseInitTypeDef EraseInitStruct;
	/* Unlock the flash;Nothing will happen if flash is not unlocked */
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

	FirstPage = Get_Flash_Page_Number(User_Flash_Start_Address);
	NoOfPages = Get_Flash_Page_Number(User_Flash_End_Address) - FirstPage +  1;

	/* Size of each page is 4Kb and stm32l432kc has only one falsh bank, Number of pages to be written */
	EraseInitStruct.TypeErase 	= FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks      	= FLASH_BANK_1;
	EraseInitStruct.Page       	= FirstPage;
	EraseInitStruct.NbPages     = NoOfPages;

	/* Perticular page address must be rased before writing any data */
	if(HAL_FLASHEx_Erase(&EraseInitStruct,&PageError) != HAL_OK)
	{
		Result = RESULT_ERROR;
		EraseSuccessful = false;
	}
	else
	{
		EraseSuccessful = true;
	}

	/* Proceed for writing the data only if erase operation is successful */
	if (EraseSuccessful == true)
	{
		Address = User_Flash_Start_Address;
		while (Address < User_Flash_End_Address)
		{
			/* Write the 8Byte i.e.64 bit data */
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address,TxBuffer[Index++]) == HAL_OK)
			{
				Address = Address + 8;
			}
			else
			{
				HAL_FLASH_Lock();
				Result = RESULT_ERROR ;
			}
		}
	}
	else
	{
		/* If there is any error while erasing the flash page then lock the flash avoid accedental data write corruptinf the flash */
		HAL_FLASH_Lock();
		Result = RESULT_ERROR ;
	}

	HAL_FLASH_Lock();

#endif
	return Result;
}

/**
 * @brief  Read data from flash of the micro controller
 * @param  User_Flash_Start_Address		: Start address of flash from where data is to be read
 * @param  User_Flash_End_Address		: End address of flash upto which data is to be read
 * @param  RxBuffer						: Pointer to the buffer to which read data is stored
 * @retval RES_ERROR					: Not Successful
 * 		   RES_OK						: Successful
 */
uint8_t MCU_Flash_Read(uint32_t User_Flash_Start_Address,uint32_t User_Flash_End_Address,uint32_t *RxBuffer)
{
	uint32_t Address = 0,Index = 0;
	uint8_t Result = RESULT_OK;
#ifdef  BMS_VERSION
	Address = User_Flash_Start_Address;
	while(Address < User_Flash_End_Address)
	{
		/* Read the 4 bytes data from the mentioned address */
		RxBuffer[Index++] = *(__IO uint32_t *)Address;
		Address += 4;
	}
#endif
	return Result;
}

/**
 * @brief  Finds the page number in which perticular address lies
 * @param  Address		: Flash address who's page number is to be found
 * @retval Page_Number	: Page number ranging from 0 - 127
 */
static uint32_t Get_Flash_Page_Number(uint32_t Address)
{
	uint32_t Page_Number = 0;
#ifdef BMS_VERSION
	if (Address < (FLASH_BASE + FLASH_BANK_SIZE))
	{
		Page_Number= (Address - FLASH_BASE) / FLASH_PAGE_SIZE;
	}
	else
	{
		Page_Number = (Address - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
	}
#endif
	return Page_Number;
}

/**
 * @brief  Deinitialize the flash module
 * @param  None
 */
void MCU_Flash_DeInit(void)
{
#ifdef BMS_VERSION

	HAL_FLASH_Lock();

#endif

}
