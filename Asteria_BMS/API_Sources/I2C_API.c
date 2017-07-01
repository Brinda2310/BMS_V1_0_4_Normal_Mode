
/******************************************************************************
 * @file    I2C_API.h
 * @author  NIKHIL INGALE
 * @date    20-Jan-2017
 * @brief   This file contains all the functions definations for the I2C
 *          module driver
 ******************************************************************************/

#include <I2C_API.h>
#include "BMS_Serial_Communication.h"
#include "BMS_Timing.h"

#if defined (USE_I2C1) || defined(USE_I2C3)
#if (I2C1_MODE == NORMAL_I2C_MODE) || (I2C3_MODE == NORMAL_I2C_MODE)
	I2C_HandleTypeDef I2CHandle[NUM_OF_I2C_BUSES];
#endif
#if I2C1_MODE == SMBUS_MODE || I2C3_MODE == SMBUS_MODE
	SMBUS_HandleTypeDef SMBus_Handle[NUM_OF_I2C_BUSES];
	static uint8_t SMBUS_Own_Address;
	static bool SMBUS_Read_Request = false,SMBUS_Write_Request = false;
    uint8_t SMBUS_RxData[20];
    static uint8_t Bytes_Count = 1;
    uint16_t SMBUS_Req_Time_Count = 0;
#endif
#endif


/**
 * @brief  Initialize the I2C bus with specified parameters
 * @param  I2C_Num			: I2C Number of the specific micro controller [I2C_1,I2C_3]
 * @param  I2C_Address		: Micro controllers I2C address
 * @param  Clock_Frequency	: Speed of I2C bus [I2C_100KHZ,I2C_400KHZ,I2C_1MHZ]
 * @param  I2C_Mode 		: Mode of I2C [I2C_MASTER,I2C_SLAVE]
 * @retval RES_ERROR		: Not Successful
 * 		   RES_OK			: Successful
 */
uint8_t I2C_Init(uint8_t I2C_Num,uint8_t I2C_Own_Address,uint32_t Clock_Frequency,uint8_t I2C_Mode)
{
	uint8_t Result = RESULT_OK;

#ifdef BMS_VERSION
	GPIO_InitTypeDef  GPIO_InitStruct;

	switch(I2C_Num)
	{
		case I2C_1:
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
			GPIO_InitStruct.Pull      = GPIO_NOPULL;
			GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		#endif

		#if I2C1_INT_MODE == ENABLE
			HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
			HAL_NVIC_SetPriority(I2C1_EV_IRQn, I2C1_SUBPRIOIRTY, I2C1_PRIOIRTY);
			HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
		#endif

		#if I2C1_MODE == NORMAL_I2C_MODE && defined(USE_I2C1)
			I2CHandle[I2C1_HANDLE_INDEX].Instance         	  = I2C1;
			I2CHandle[I2C1_HANDLE_INDEX].Init.Timing  		  = Clock_Frequency;
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
		#elif I2C1_MODE == SMBUS_MODE && defined(USE_I2C1)
			SMBUS_Own_Address = I2C_Own_Address;

			SMBus_Handle[I2C1_HANDLE_INDEX].Instance		 	  = I2C1;
			SMBus_Handle[I2C1_HANDLE_INDEX].Init.Timing 		  = Clock_Frequency;
			SMBus_Handle[I2C1_HANDLE_INDEX].Init.AnalogFilter 	  = SMBUS_ANALOGFILTER_ENABLE;
			SMBus_Handle[I2C1_HANDLE_INDEX].Init.OwnAddress1  	  = I2C_Own_Address;
			SMBus_Handle[I2C1_HANDLE_INDEX].Init.AddressingMode   = SMBUS_ADDRESSINGMODE_7BIT;
			SMBus_Handle[I2C1_HANDLE_INDEX].Init.DualAddressMode  = SMBUS_DUALADDRESS_DISABLED;
			SMBus_Handle[I2C1_HANDLE_INDEX].Init.OwnAddress2 	  = 0;
			SMBus_Handle[I2C1_HANDLE_INDEX].Init.OwnAddress2Masks = SMBUS_OA2_NOMASK;
			SMBus_Handle[I2C1_HANDLE_INDEX].Init.GeneralCallMode  = SMBUS_GENERALCALL_DISABLED;
			SMBus_Handle[I2C1_HANDLE_INDEX].Init.NoStretchMode    = SMBUS_NOSTRETCH_DISABLED;
			SMBus_Handle[I2C1_HANDLE_INDEX].Init.PacketErrorCheckMode = SMBUS_PEC_DISABLED;
			if(I2C_Mode == I2C_SLAVE)
				SMBus_Handle[I2C1_HANDLE_INDEX].Init.PeripheralMode  = SMBUS_PERIPHERAL_MODE_SMBUS_SLAVE;
			else
				SMBus_Handle[I2C1_HANDLE_INDEX].Init.PeripheralMode  = SMBUS_PERIPHERAL_MODE_SMBUS_HOST;
			SMBus_Handle[I2C1_HANDLE_INDEX].Init.SMBusTimeout 	  = 0;

			if(HAL_SMBUS_Init(&SMBus_Handle[I2C1_HANDLE_INDEX]) != HAL_OK)
				Result = RESULT_ERROR;

		#endif

		break;

		case I2C_3:
			/* Enable the I2C3 peripheral clock */
			__HAL_RCC_I2C3_CLK_ENABLE();

			/* Enable the clocks I2C3 port pins
			 * Configure the pins for alternate functions PA7(I2C_SCL), PB4(I2C_SDA)*/
			__HAL_RCC_GPIOB_CLK_ENABLE();
			__HAL_RCC_GPIOA_CLK_ENABLE();

			GPIO_InitStruct.Pin       = GPIO_PIN_4;
			GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
			GPIO_InitStruct.Pull      = GPIO_NOPULL;
			GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

			GPIO_InitStruct.Pin       = GPIO_PIN_7;
			GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
			GPIO_InitStruct.Pull      = GPIO_NOPULL;
			GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		#if I2C3_INT_MODE == ENABLE
			HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
			HAL_NVIC_SetPriority(I2C3_EV_IRQn, I2C3_SUBPRIOIRTY, I2C3_PRIOIRTY);
			HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);
		#endif

		#if	I2C3_MODE == NORMAL_I2C_MODE && defined(USE_I2C3)
			I2CHandle[I2C3_HANDLE_INDEX].Instance         	  = I2C3;
			I2CHandle[I2C3_HANDLE_INDEX].Init.Timing 		  = I2C_400KHZ;
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
		#elif I2C3_MODE == SMBUS_MODE && defined(USE_I2C3)
			SMBUS_Own_Address = I2C_Own_Address;

			SMBus_Handle[I2C3_HANDLE_INDEX].Instance		 	  = I2C3;
			SMBus_Handle[I2C3_HANDLE_INDEX].Init.Timing 		  = Clock_Frequency;
			SMBus_Handle[I2C3_HANDLE_INDEX].Init.AnalogFilter 	  = SMBUS_ANALOGFILTER_ENABLE;
			SMBus_Handle[I2C3_HANDLE_INDEX].Init.OwnAddress1  	  = I2C_Own_Address;
			SMBus_Handle[I2C3_HANDLE_INDEX].Init.AddressingMode   = SMBUS_ADDRESSINGMODE_7BIT;
			SMBus_Handle[I2C3_HANDLE_INDEX].Init.DualAddressMode  = SMBUS_DUALADDRESS_DISABLED;
			SMBus_Handle[I2C3_HANDLE_INDEX].Init.OwnAddress2 	  = 0;
			SMBus_Handle[I2C3_HANDLE_INDEX].Init.OwnAddress2Masks = SMBUS_OA2_NOMASK;
			SMBus_Handle[I2C3_HANDLE_INDEX].Init.GeneralCallMode  = SMBUS_GENERALCALL_DISABLED;
			SMBus_Handle[I2C3_HANDLE_INDEX].Init.NoStretchMode    = SMBUS_NOSTRETCH_DISABLED;
			SMBus_Handle[I2C3_HANDLE_INDEX].Init.PacketErrorCheckMode = SMBUS_PEC_DISABLED;
			if(I2C_Mode == I2C_SLAVE)
				SMBus_Handle[I2C3_HANDLE_INDEX].Init.PeripheralMode  = SMBUS_PERIPHERAL_MODE_SMBUS_SLAVE;
			else
				SMBus_Handle[I2C3_HANDLE_INDEX].Init.PeripheralMode  = SMBUS_PERIPHERAL_MODE_SMBUS_HOST;
			SMBus_Handle[I2C3_HANDLE_INDEX].Init.SMBusTimeout 	  = 0;

			if(HAL_SMBUS_Init(&SMBus_Handle[I2C3_HANDLE_INDEX]) != HAL_OK)
				Result = RESULT_ERROR;
		#endif
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
		#if I2C1_MODE == NORMAL_I2C_MODE && defined(USE_I2C1)
			if(HAL_I2C_DeInit(&I2CHandle[I2C1_HANDLE_INDEX]) != HAL_OK)
				Result = RESULT_ERROR;
		#elif I2C1_MODE == SMBUS_MODE && defined(USE_I2C1)
			if(HAL_SMBUS_DeInit(&SMBus_Handle[I2C1_HANDLE_INDEX]) != HAL_OK)
				Result = RESULT_ERROR;
		#endif
			break;
		case I2C_3:
		#if I2C3_MODE == NORMAL_I2C_MODE && defined(USE_I2C3)
			if(HAL_I2C_DeInit(&I2CHandle[I2C3_HANDLE_INDEX]) != HAL_OK)
				Result = RESULT_ERROR;
		#elif I2C3_MODE == SMBUS_MODE && defined(USE_I2C3)
			if(HAL_SMBUS_DeInit(&SMBus_Handle[I2C3_HANDLE_INDEX]) != HAL_OK)
				Result = RESULT_ERROR;
		#endif
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
		#if I2C1_MODE == NORMAL_I2C_MODE && defined(USE_I2C1)
			if (HAL_I2C_Master_Transmit(&I2CHandle[I2C1_HANDLE_INDEX], Dev_Address,	TxBuffer, Size, I2C_DATA_TIMEOUT) != HAL_OK)
				Result = RESULT_ERROR;
		#endif
			break;
		case I2C_3:
		#if I2C3_MODE == NORMAL_I2C_MODE && defined(USE_I2C3)
			if (HAL_I2C_Master_Transmit(&I2CHandle[I2C3_HANDLE_INDEX], Dev_Address,	TxBuffer, Size, I2C_DATA_TIMEOUT) != HAL_OK)
				Result = RESULT_ERROR;
		#endif
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
 * @param  Dev_Address		: Device address from which data is to be received
 * @param  RxBuffer			: Pointer to the data buffer in which data is to be stored
 * @param  Size				: Size of data buffer to be received
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
		#if I2C1_MODE == NORMAL_I2C_MODE && defined(USE_I2C1)
			if (HAL_I2C_Master_Receive(&I2CHandle[I2C1_HANDLE_INDEX],Dev_Address | 0x01, RxBuffer, Size, I2C_DATA_TIMEOUT) != HAL_OK)
				Result = RESULT_ERROR;
		#endif
			break;

		case I2C_3:
		#if I2C3_MODE == NORMAL_I2C_MODE && defined(USE_I2C3)
			if (HAL_I2C_Master_Receive(&I2CHandle[I2C3_HANDLE_INDEX],Dev_Address | 0x01, RxBuffer, Size, I2C_DATA_TIMEOUT) != HAL_OK)
				Result = RESULT_ERROR;
		#endif
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
#if I2C1_MODE == NORMAL_I2C_MODE
  HAL_I2C_EV_IRQHandler(&I2CHandle[I2C1_HANDLE_INDEX]);
#elif I2C3_MODE == SMBUS_MODE
  HAL_SMBUS_EV_IRQHandler(&SMBus_Handle[I2C1_HANDLE_INDEX]);
#endif
}

#endif /* I2C1_INT_MODE end*/

#if I2C3_INT_MODE == ENABLE

void I2C3_EV_IRQHandler(void)
{
#if I2C3_MODE == NORMAL_I2C_MODE
  HAL_I2C_EV_IRQHandler(&I2CHandle[I2C3_HANDLE_INDEX]);
#elif I2C3_MODE == SMBUS_MODE
  HAL_SMBUS_EV_IRQHandler(&SMBus_Handle[I2C3_HANDLE_INDEX]);
#endif
}

void HAL_SMBUS_ListenCpltCallback(SMBUS_HandleTypeDef *hsmbus)
{
	HAL_SMBUS_EnableListen_IT(&SMBus_Handle[I2C3_HANDLE_INDEX]);
}

void HAL_SMBUS_SlaveTxCpltCallback(SMBUS_HandleTypeDef *hsmbus)
{
	HAL_SMBUS_EnableListen_IT(&SMBus_Handle[I2C3_HANDLE_INDEX]);
}

void HAL_SMBUS_SlaveRxCpltCallback(SMBUS_HandleTypeDef *hsmbus)
{
	/* This function will called when all the bytes mentioned in the address callback function
	 * are received */
	SMBUS_Read_Request = true;
	HAL_SMBUS_EnableListen_IT(&SMBus_Handle[I2C3_HANDLE_INDEX]);
}

void HAL_SMBUS_AddrCallback(SMBUS_HandleTypeDef *hsmbus, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	AddrMatchCode = AddrMatchCode << 1;
	if(AddrMatchCode == (uint16_t)SMBUS_Own_Address)
	{
		/* Read request (AP requests BMS to send the data as per the packet index */
		if(TransferDirection == 0)
		{
			HAL_SMBUS_Slave_Receive_IT(hsmbus, &SMBUS_RxData[0], Bytes_Count, SMBUS_AUTOEND_MODE);
		}
		/* Write request */
		else
		{
			SMBUS_Write_Request = true;
		}
	}
}
#endif /* I2C3_INT_MODE end*/

void SMBUS_Enable_Listen_Mode(uint8_t I2C_Num)
{
	switch (I2C_Num)
	{
		case I2C_1:
		#if I2C1_MODE == SMBUS_MODE && defined(USE_I2C1)
			HAL_SMBUS_EnableListen_IT(&SMBus_Handle[I2C1_HANDLE_INDEX]);
		#endif
			break;
		case I2C_3:
		#if I2C3_MODE == SMBUS_MODE && defined(USE_I2C3)
			HAL_SMBUS_EnableListen_IT(&SMBus_Handle[I2C3_HANDLE_INDEX]);
		#endif
			break;
		default:
			break;
	}
}

uint8_t SMBUS_Request_Check(uint8_t *RxBuffer)
{
	uint8_t Index = 0;
	uint8_t Result = 255;

#if I2C3_MODE == SMBUS_MODE || I2C1_MODE == SMBUS_MODE
	/* This flag will be true only when the MCU receives all the bytes mentioned as the count in the
	 * address complete callback function */
	if(_30_Hz_SMBUS_Flag == true)
	{
		SMBUS_Req_Time_Count++;
		_30_Hz_SMBUS_Flag = false;
	}

	if(SMBUS_Read_Request == true)
	{
		for(Index = 0;Index < Bytes_Count;Index++)
		{
			*RxBuffer++ = SMBUS_RxData[Index];
		}
		SMBUS_Read_Request = false;
		SMBUS_Req_Time_Count = 0;
		Result = SMBUS_REQ_SUCCESSFUL;
	}

	if(SMBUS_Req_Time_Count > 100)
	{
		SMBUS_Req_Time_Count = 0;
		Result = SMBUS_REQ_TIMEOUT;
	}
#endif
	return Result;
}

void SMBUS_Serve_Request(uint8_t *TxBuffer,uint8_t Size)
{
#if I2C3_MODE == SMBUS_MODE || I2C1_MODE == SMBUS_MODE
	/* This flag will be true only when there is any request for sending any data (from BMS) to other
	 * device (in our case its AP, write mode) */
	if(SMBUS_Write_Request == true)
	{
		SMBUS_Write_Request = false;
		HAL_SMBUS_Slave_Transmit_IT(&SMBus_Handle[I2C3_HANDLE_INDEX],TxBuffer,Size,SMBUS_FIRST_AND_LAST_FRAME_NO_PEC);
		Delay_Millis(5);
	}
#endif
}

/* Function to set the number of bytes to be received from AP in the SMBus address call back routine */
void Set_Bytes_Count(uint8_t *Count)
{
	Bytes_Count = *Count;
}
#endif /* BMS_VERSION end */

