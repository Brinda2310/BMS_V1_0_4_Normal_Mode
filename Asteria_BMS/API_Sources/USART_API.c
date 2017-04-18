 /******************************************************************************
  * @file    USART_API.c
  * @author  NIKHIL INGALE
  * @date    18-Jan-2017
  * @brief   This file contains all the functions definations for the USART
  *          module driver which can be used any STM micro controllers.
  *          User has to define the controller in Hardware_Config.h file.
  ******************************************************************************/

#include "USART_API.h"

#ifdef BMS_VERSION
	/* USART Structure defined in HAL layer and the same is used for handling the read/write operations
	 * Number of USARTs are defined in Hardware_Config.h file */
	UART_HandleTypeDef USARTHandle[NUM_OF_USARTS];
#endif

/**
 * @brief  Initialize the USART according to the specified parameters
 * @param  USART_Num	: USART Number of the specific micro controller [USART_1,USART_2]
 * @param  Baud_Rate	: Baud Rate of USART e.g. 57600, 115200 etc
 * @retval RES_ERROR	: Not Successful
 * 		   RES_OK		: Successful
 */
uint8_t USART_Init(uint8_t USART_Num,int32_t Baud_Rate)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION
	GPIO_InitTypeDef  GPIO_InitStruct;

	switch(USART_Num)
	{
	case USART_1:

		/* If USART1_REMAP is enabled in Hadware_Config.h file then use the respective pins */
		#if USART1_REMAP == ENABLE

			/* Enable the clocks for USART1 and port pins
			 * Configure the pins for alternate functions PB6(TX) and PB7(RX)*/
			__HAL_RCC_GPIOB_CLK_ENABLE();
			__HAL_RCC_USART1_CLK_ENABLE();

			GPIO_InitStruct.Pin       = GPIO_PIN_6 | GPIO_PIN_7;
			GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull      = GPIO_PULLUP;
			GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF7_USART1;

			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		#else
			/* Enable the clocks for USART1 and port pins
			 * Configure the pins for alternate functions PA9(TX) and PA10(RX) */
			__HAL_RCC_GPIOA_CLK_ENABLE();
			__HAL_RCC_USART1_CLK_ENABLE();

			GPIO_InitStruct.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
			GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull      = GPIO_PULLUP;
			GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		#endif

		/* If USART1_INT_MODE macro is enabled then set the priorities for USART1 handler and enable the same
		 * Priorities are defined in Hardware_Config.h file*/
		#if USART1_INT_MODE == ENABLE
			#if (USART1_PRIORITY >=4 && USART1_PRIORITY <= 7)
				HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
			#elif (USART1_PRIORITY >=8 && USART1_PRIORITY <= 15)
				HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
			#else
				HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
			#endif
			HAL_NVIC_SetPriority(USART1_IRQn, USART1_PRIORITY, USART1_SUBPRIORITY);
			HAL_NVIC_EnableIRQ(USART1_IRQn);
		#endif

		USARTHandle[USART1_HANDLE_INDEX].Instance        = USART1;
		USARTHandle[USART1_HANDLE_INDEX].Init.BaudRate   = Baud_Rate;
		USARTHandle[USART1_HANDLE_INDEX].Init.WordLength = UART_WORDLENGTH_8B;
		USARTHandle[USART1_HANDLE_INDEX].Init.StopBits   = UART_STOPBITS_1;
		USARTHandle[USART1_HANDLE_INDEX].Init.Parity     = UART_PARITY_NONE;
		USARTHandle[USART1_HANDLE_INDEX].Init.HwFlowCtl  = UART_HWCONTROL_NONE;
		USARTHandle[USART1_HANDLE_INDEX].Init.Mode       = UART_MODE_TX_RX;

		/* Initialize the USART1 with the configuration parameters */
		if(HAL_UART_Init(&USARTHandle[USART1_HANDLE_INDEX]) != HAL_OK)
			Result = RESULT_ERROR;
		break;

	case USART_2:

		/* Enable the clocks for USART2 and port pins
		 * Configure the pins for alternate functions PA2(TX) and PA3(RX)*/
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_USART2_CLK_ENABLE();

		GPIO_InitStruct.Pin       = GPIO_PIN_2 | GPIO_PIN_3;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* If USART2_INT_MODE macro is enabled then set the priorities for USART2 handler and enable the same
		 * Priorities are defined in Hardware_Config.h file*/
		#if USART2_INT_MODE == ENABLE
			#if (USART2_PRIORITY >=4 && USART2_PRIORITY <= 7)
				HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
			#elif (USART2_PRIORITY >=8 && USART2_PRIORITY <= 15)
				HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
			#else
				HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
			#endif
			HAL_NVIC_SetPriority(USART2_IRQn, USART2_PRIORITY, USART2_SUBPRIORITY);
			HAL_NVIC_EnableIRQ(USART2_IRQn);
		#endif

		USARTHandle[USART2_HANDLE_INDEX].Instance        = USART2;
		USARTHandle[USART2_HANDLE_INDEX].Init.BaudRate   = Baud_Rate;
		USARTHandle[USART2_HANDLE_INDEX].Init.WordLength = UART_WORDLENGTH_8B;
		USARTHandle[USART2_HANDLE_INDEX].Init.StopBits   = UART_STOPBITS_1;
		USARTHandle[USART2_HANDLE_INDEX].Init.Parity     = UART_PARITY_NONE;
		USARTHandle[USART2_HANDLE_INDEX].Init.HwFlowCtl  = UART_HWCONTROL_NONE;
		USARTHandle[USART2_HANDLE_INDEX].Init.Mode       = UART_MODE_TX_RX;

		/* Initialize the USART2 with the configuration parameters */
		if(HAL_UART_Init(&USARTHandle[USART2_HANDLE_INDEX]) != HAL_OK)
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
 * @brief  Deinitialize the USART according to the specified parameter
 * @param  USART_Num	: USART Number of the specific micro controller [USART_1,USART_2]
 * @retval RES_ERROR	: Not Successful
 * 		   RES_OK		: Successful
 */
uint8_t USART_DeInit(uint8_t USART_Num)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION

	switch (USART_Num)
	{
		case USART_1:
			if(HAL_USART_DeInit((USART_HandleTypeDef*)&USARTHandle[USART1_HANDLE_INDEX]) != HAL_OK)
				Result = RESULT_ERROR;
			break;
		case USART_2:
			if(HAL_USART_DeInit((USART_HandleTypeDef*)&USARTHandle[USART2_HANDLE_INDEX]) != HAL_OK)
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
 * @brief  Transmit data to over USART
 * @param  USART_Num	: USART Number of specific micro controller [USART_1,USART_2]
 * @param  TxBuffer		: Pointer to the data buffer to be trasnmitted
 * @param  Size			: Size of Data to be transmitted over USART
 * @retval RES_ERROR	: Not Successful
 * 		   RES_OK		: Successful
 */
uint8_t USART_Write(uint8_t USART_Num, void *TxBuffer,uint16_t Size)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION
	switch (USART_Num)
	{
		case USART_1:
			if(HAL_UART_Transmit_IT(&USARTHandle[USART1_HANDLE_INDEX], (uint8_t*)TxBuffer, Size) != HAL_OK)
				Result = RESULT_ERROR;
			break;
		case USART_2:
			if(HAL_UART_Transmit_IT(&USARTHandle[USART2_HANDLE_INDEX], (uint8_t*)TxBuffer, Size) != HAL_OK)
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
 * @brief  Recieve data through USART and store it in buffer specified in the parameter
 * @param  USART_Num	: USART Number of specific micro controller [USART_1,USART_2]
 * @param  RxBufffer	: Pointer to the data buffer in which data to be recieved
 * @param  Size			: Size of Data to be recieved over USART
 * @retval RES_ERROR	: Not Successful
 * 		   RES_OK		: Successful
 */
uint8_t USART_Read(uint8_t USART_Num, void *RxBufffer,uint16_t Size)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION
	switch (USART_Num)
	{
		case USART_1:
			if(HAL_UART_Receive_IT(&USARTHandle[USART1_HANDLE_INDEX], (uint8_t*)RxBufffer, Size) != HAL_OK)
				Result = RESULT_ERROR;
			break;
		case USART_2:
			if(HAL_UART_Receive_IT(&USARTHandle[USART2_HANDLE_INDEX], (uint8_t*)RxBufffer, Size) != HAL_OK)
				Result = RESULT_ERROR;
			break;
		default:
			break;
	}

#endif
return Result;
}

/*
 *  @brief	USART Handlers for the specific micro controllers can be defined in this section
 *			Here the defined handlers are for stm32l432kc micro controller
 * */
#ifdef BMS_VERSION
void USART1_IRQHandler(void)
{
	HAL_UART_IRQHandler(&USARTHandle[USART1_HANDLE_INDEX]);
}

void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&USARTHandle[USART2_HANDLE_INDEX]);
}
#endif


