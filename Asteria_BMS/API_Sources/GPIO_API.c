
 /******************************************************************************
  * @file    GPIO_API.c
  * @author  NIKHIL INGALE
  * @date    27-Jan-2017
  * @brief   This file contains all the functions definations for the GPIO
  *          module driver which can be used any STM micro controllers.
  *          User has to define the controller in Hardware_Config.h file.
  ******************************************************************************/

#include "GPIO_API.h"

/**
 * @brief  Initialize the GPIO according to the specified parameters
 * @param  GPIO_Port		: Port Number of the specific micro controller [GPIO_A,GPIOA_B]
 * @param  GPIO_Pin			: GPIO port pin to be configured (For boards it can be defined in Hardware_Config.h file)
 * @param  Mode 			: GPIO mode [GPIO_INPUT,GPIO_OUTPUT or GPIO_FLOATING]
 * @param  GPIO_Init_State 	: GPIO pin's initial state [PULLUP,PULLDOWN or NOPULL]
 * @retval RES_ERROR		: Not Successful
 * 		   RES_OK			: Successful
 */
uint8_t GPIO_Init(uint8_t GPIO_Port,uint32_t GPIO_Pin,uint8_t Mode,uint8_t GPIO_Init_State)
{
	uint8_t Result = RESULT_OK;

#ifdef BMS_VERSION
	GPIO_InitTypeDef  GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_Pin ;

	if (GPIO_Port == GPIO_A || GPIO_Port == GPIO_B)
	{
		if (Mode == GPIO_INPUT)
			GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		else if (Mode == GPIO_FLOATING)
			GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
		else
			GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

		if (GPIO_Init_State == PULLDOWN)
			GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		else if (GPIO_Init_State == NOPULL)
			GPIO_InitStruct.Pull = GPIO_NOPULL;
		else
			GPIO_InitStruct.Pull = GPIO_PULLUP;

		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	}
	else
		Result = RESULT_ERROR;

	switch(GPIO_Port)
	{
		case GPIO_A:
			__HAL_RCC_GPIOA_CLK_ENABLE();
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		break;
		case GPIO_B:
			__HAL_RCC_GPIOB_CLK_ENABLE();
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		break;
		default:
			Result = RESULT_ERROR;
		break;
	}
#endif
return Result;
}

/**
 * @brief  Write the status to GPIO pin according to the specified parameters
 * @param  GPIO_Port		: Port number of the specific micro controller
 * @param  GPIO_Pin			: GPIO port pin to be configured (For boards it can be defined in Hardware_Config.h file)
 * @param  Mode 			: GPIO mode GPIO_INPUT,GPIO_OUTPUT or GPIO_FLOATING
 * @param  GPIO_Init_State 	: GPIO pin's initial state PULLUP,PULLDOWN or NOPULL
 * @retval RES_ERROR		: Not Successful
 * 		   RES_OK			: Successful
 */
uint8_t GPIO_Write(uint8_t GPIO_Port,uint32_t GPIO_Pin,uint8_t GPIO_State)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION
	switch(GPIO_Port)
	{
		case GPIO_A:
			if(GPIO_State == PIN_TOGGLE)
				HAL_GPIO_TogglePin(GPIOA,GPIO_Pin);
			else
				HAL_GPIO_WritePin(GPIOA,GPIO_Pin,GPIO_State);
			break;
		case GPIO_B:
			if(GPIO_State == PIN_TOGGLE)
				HAL_GPIO_TogglePin(GPIOB,GPIO_Pin);
			else
				HAL_GPIO_WritePin(GPIOB,GPIO_Pin,GPIO_State);
			break;
		default :
			Result = RESULT_ERROR;
			break;
	}
#endif
return Result;
}

/**
 * @brief  Read the status to GPIO pin according to the specified parameters
 * @param  GPIO_Port		: Port number of the specific micro controller
 * @param  GPIO_Pin			: GPIO port pin of which status is to be read (For boards it can be defined in Hardware_Config.h file)
 * @retval RES_ERROR		: Not Successful
 * 		   RES_OK			: Successful
 */
uint8_t GPIO_Read(uint8_t GPIO_Port,uint32_t GPIO_Pin)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION
	switch(GPIO_Port)
	{
		case GPIO_A:
			if(HAL_GPIO_ReadPin(GPIOA,(uint16_t)GPIO_Pin) == (uint32_t)GPIO_PIN_SET)
				return PIN_HIGH;
			else
				return PIN_LOW;
		break;
		case GPIO_B:
			if(HAL_GPIO_ReadPin(GPIOB,(uint16_t)GPIO_Pin) == (uint32_t)GPIO_PIN_SET)
				return PIN_HIGH;
			else
				return PIN_LOW;
		break;
		default:
			Result = RESULT_ERROR;
		break;
	}

#endif
	return Result;
}

/**
 * @brief  Deinitialize the GPIO pin according to the specified parameters
 * @param  GPIO_Port		: Port number of the specific micro controller
 * @param  GPIO_Pin			: GPIO port pin to be deinitialized (For boards it can be defined in Hardware_Config.h file)
 * @retval RES_ERROR		: Not Successful
 * 		   RES_OK			: Successful
 */
uint8_t GPIO_DeInit(uint8_t GPIO_Port,uint32_t GPIO_Pin)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION

	switch (GPIO_Port)
	{
		case GPIO_A:
			HAL_GPIO_DeInit(GPIOA,GPIO_Pin);
		break;

		case GPIO_B:
			HAL_GPIO_DeInit(GPIOB,GPIO_Pin);
		break;

		default:
			Result = RESULT_ERROR;
		break;
	}
#endif
	return Result;
}

