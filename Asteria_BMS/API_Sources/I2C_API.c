
/******************************************************************************
 * @file    I2C_API.h
 * @author  NIKHIL INGALE
 * @date    20-Jan-2017
 * @brief   This file contains all the functions definations for the I2C
 *          module driver
 ******************************************************************************/

#include "I2C_API.h"

#ifdef BMS_VERSION
	I2C_HandleTypeDef I2CHandle[NUM_OF_I2C_BUSES];
#endif

/**
 * @brief  Initialize the I2C bus with specified parameters
 * @param  I2C_Num			: I2C Number of the specific micro controller [I2C_1,I2C_3]
 * @param  I2C_Address		: Micro controllers I2C adddress
 * @param  Clock_Frequency	: Speed of I2C bus [I2C_100KHZ,I2C_400KHZ,I2C_1MHZ]
 * @retval RES_ERROR		: Not Successful
 * 		   RES_OK			: Successful
 */
uint8_t I2C_Init(uint8_t I2C_Num,uint8_t I2C_Own_Address,uint32_t Clock_Frequency)
{
	uint8_t Result = RESULT_OK;

#ifdef BMS_VERSION
	GPIO_InitTypeDef  GPIO_InitStruct;
	RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct;

	switch(I2C_Num)
	{
		case I2C_1:
			/* Select the clock source for I2C1 bus */
			RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
			RCC_PeriphCLKInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
			HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

			/* Enable the I2C1 peripheral clock */
			__HAL_RCC_I2C1_CLK_ENABLE();

			/* If I2C_REMAP is enabled the choose the pins accordingly */
		#if I2C1_REMAP == ENABLE
			/* Enable the clocks I2C1 port pins
			 * Configure the pins for alternate functions PB6(I2C_SCL), PB7(I2C_SDA)*/
			HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
			__HAL_RCC_GPIOB_CLK_ENABLE();

			GPIO_InitStruct.Pin       = GPIO_PIN_6 | GPIO_PIN_7;
			GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
			GPIO_InitStruct.Pull      = GPIO_PULLUP;
			GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		#else
			/* Enable the clocks I2C1 port pins
			 * Configure the pins for alternate functions PA9(I2C_SCL), PA10(I2C_SDA)*/
			__HAL_RCC_GPIOA_CLK_ENABLE();
			GPIO_InitStruct.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
			GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
			GPIO_InitStruct.Pull      = GPIO_PULLUP;
			GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		#endif

			I2CHandle[I2C1_HANDLE_INDEX].Instance             = I2C1;

			if(Clock_Frequency == I2C_400KHZ)
				I2CHandle[I2C1_HANDLE_INDEX].Init.Timing  = I2C_400KHZ;
			else if(Clock_Frequency == I2C_1MHZ)
				I2CHandle[I2C1_HANDLE_INDEX].Init.Timing  = I2C_1MHZ;
			else
				I2CHandle[I2C1_HANDLE_INDEX].Init.Timing  = I2C_100KHZ;

			I2CHandle[I2C1_HANDLE_INDEX].Init.OwnAddress1     = I2C_Own_Address;
			I2CHandle[I2C1_HANDLE_INDEX].Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
			I2CHandle[I2C1_HANDLE_INDEX].Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
			I2CHandle[I2C1_HANDLE_INDEX].Init.OwnAddress2     = 0;
			I2CHandle[I2C1_HANDLE_INDEX].Init.OwnAddress2Masks = I2C_OA2_NOMASK;
			I2CHandle[I2C1_HANDLE_INDEX].Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
			I2CHandle[I2C1_HANDLE_INDEX].Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

			/* Initialize the I2C1 with the configuration parameters */
			if(HAL_I2C_Init(&I2CHandle[I2C1_HANDLE_INDEX]) != HAL_OK)
				Result = RESULT_ERROR;

			if(HAL_I2CEx_ConfigAnalogFilter(&I2CHandle[I2C1_HANDLE_INDEX],I2C_ANALOGFILTER_ENABLE) != HAL_OK)
				Result = RESULT_ERROR;

		break;

		case I2C_3:
			/* Select the clock source for I2C3 bus */
			RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C3;
			RCC_PeriphCLKInitStruct.I2c1ClockSelection = RCC_I2C3CLKSOURCE_SYSCLK;
			HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

			/* Enable the I2C3 peripheral clock */
			__HAL_RCC_I2C3_CLK_ENABLE();

			/* Enable the clocks I2C3 port pins */
			__HAL_RCC_GPIOB_CLK_ENABLE();
			__HAL_RCC_GPIOA_CLK_ENABLE();

			GPIO_InitStruct.Pin       = GPIO_PIN_4;
			GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
			GPIO_InitStruct.Pull      = GPIO_PULLUP;
			GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

			GPIO_InitStruct.Pin       = GPIO_PIN_7;
			GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
			GPIO_InitStruct.Pull      = GPIO_PULLUP;
			GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

			I2CHandle[I2C3_HANDLE_INDEX].Instance         = I2C3;

			if(Clock_Frequency== I2C_400KHZ)
				I2CHandle[I2C3_HANDLE_INDEX].Init.Timing  = I2C_400KHZ;
			else if(Clock_Frequency == I2C_1MHZ)
				I2CHandle[I2C3_HANDLE_INDEX].Init.Timing  = I2C_1MHZ;
			else
				I2CHandle[I2C3_HANDLE_INDEX].Init.Timing  = I2C_100KHZ;

			I2CHandle[I2C3_HANDLE_INDEX].Init.OwnAddress1     = I2C_Own_Address;
			I2CHandle[I2C3_HANDLE_INDEX].Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
			I2CHandle[I2C3_HANDLE_INDEX].Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
			I2CHandle[I2C3_HANDLE_INDEX].Init.OwnAddress2     = 0;
			I2CHandle[I2C3_HANDLE_INDEX].Init.OwnAddress2Masks = I2C_OA2_NOMASK;
			I2CHandle[I2C3_HANDLE_INDEX].Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
			I2CHandle[I2C3_HANDLE_INDEX].Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

			/* Initialize the I2C3 with the configuration parameters */
			if(HAL_I2C_Init(&I2CHandle[I2C3_HANDLE_INDEX]) != HAL_OK)
				Result = RESULT_ERROR;

			if(HAL_I2CEx_ConfigAnalogFilter(&I2CHandle[I2C3_HANDLE_INDEX],I2C_ANALOGFILTER_ENABLE) != HAL_OK)
				Result = RESULT_ERROR;

			break;

		default:
			Result = RESULT_ERROR;
			break;
		}
#endif
	return Result;
}

/**
 * @brief  Deinitialize the I2C bus with specified parameter
 * @param  I2C_Num			: I2C Number of the specific micro controller [I2C_1,I2C_3]
 * @retval RES_ERROR		: Not Successful
 * 		   RES_OK			: Successful
 */
uint8_t I2C_DeInit(uint8_t I2C_Num)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION
	switch (I2C_Num)
	{
		case I2C_1:
			if(HAL_I2C_DeInit(&I2CHandle[I2C1_HANDLE_INDEX]))
				Result = RESULT_ERROR;
			break;

		case I2C_3:
			if(HAL_I2C_DeInit(&I2CHandle[I2C3_HANDLE_INDEX]))
				Result = RESULT_ERROR;
			break;

		default:
			Result = RESULT_ERROR;
			break;
	}
#endif
return Result;
}

/**
 * @brief  Transfer data over the I2C bus with specified parameters
 * @param  I2C_Num			: I2C Number of the specific micro controller [I2C_1,I2C_3]
 * @param  Dev_Address		: Device address to which data is to be transferred
 * @param  TxBuffer			: Pointer to the data buffer to be sent
 * @param  Size				: Size of data buffer to be sent
 * @retval RES_ERROR		: Not Successful
 * 		   RES_OK			: Successful
 */
uint8_t I2C_WriteData(uint8_t I2C_Num,uint16_t Dev_Address,uint8_t *TxBuffer,uint16_t Size)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION
	switch (I2C_Num)
	{
		case I2C_1:
			if (HAL_I2C_Master_Transmit(&I2CHandle[I2C1_HANDLE_INDEX], Dev_Address,	TxBuffer, Size, I2C_DATA_TIMEOUT) != HAL_OK)
				Result = RESULT_ERROR;
			break;
		case I2C_3:
			if (HAL_I2C_Master_Transmit(&I2CHandle[I2C3_HANDLE_INDEX], Dev_Address,	TxBuffer, Size, I2C_DATA_TIMEOUT) != HAL_OK)
				Result = RESULT_ERROR;
			break;

		default:
			Result = RESULT_ERROR;
			break;
	}
#endif
	return Result;
}

/**
 * @brief  Recieve data over the I2C bus with specified parameters
 * @param  I2C_Num			: I2C Number of the specific micro controller [I2C_1,I2C_3]
 * @param  Dev_Address		: Device address from which data is to be recieved
 * @param  RxBuffer			: Pointer to the data buffer in which data is to be stored
 * @param  Size				: Size of data buffer to be recieved
 * @retval RES_ERROR		: Not Successful
 * 		   RES_OK			: Successful
 */
uint8_t I2C_ReadData(uint8_t I2C_Num,uint16_t Dev_Address,uint8_t *RxBuffer,uint8_t Size)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION
	switch (I2C_Num)
	{
		case I2C_1:
			if (HAL_I2C_Master_Receive(&I2CHandle[I2C1_HANDLE_INDEX],Dev_Address | 0x01, RxBuffer, Size, I2C_DATA_TIMEOUT) != HAL_OK)
				Result = RESULT_ERROR;
			break;

		case I2C_3:
			if (HAL_I2C_Master_Receive(&I2CHandle[I2C3_HANDLE_INDEX],Dev_Address | 0x01, RxBuffer, Size, I2C_DATA_TIMEOUT) != HAL_OK)
				Result = RESULT_ERROR;
			break;

		default:
			Result = RESULT_ERROR;
			break;
	}
#endif
	return Result;
}

/*
 *  @brief	I2C Handlers for the specific micro controllers can be defined in this section
 *			Here the defined handlers are for stm32l432kc micro controller
 * */
#ifdef BMS_VERSION

#if I2C1_INT_MODE == ENABLE

void I2C1_EV_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(&I2CHandle[I2C1_HANDLE_INDEX]);
}

void I2C1_ER_IRQHandler(void)
{
  HAL_I2C_ER_IRQHandler(&I2CHandle[I2C1_HANDLE_INDEX]);
}

#endif

#if I2C3_INT_MODE == ENABLE

void I2C3_EV_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(&I2CHandle[I2C3_HANDLE_INDEX]);
}

void I2C3_ER_IRQHandler(void)
{
  HAL_I2C_ER_IRQHandler(&I2CHandle[I2C3_HANDLE_INDEX]);
}

#endif

#endif

