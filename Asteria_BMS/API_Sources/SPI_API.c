/*
 * SPI_API.c
 *
 *  Created on: 25-Jan-2017
 *      Author: NIKHIL
 */
#include "SPI_API.h"

#ifdef BMS_VERSION
	SPI_HandleTypeDef SPIHandle[NUM_OF_SPI_BUSES];
#endif

void SPI_Init(uint8_t SPI_Num,uint8_t Prescalar,uint8_t SPI_Mode)
{
#ifdef BMS_VERSION
	GPIO_InitTypeDef  GPIO_InitStruct;
	switch(SPI_Num)
	{
		case SPI_1:
			#if SPI1_REMAP == DISABLE
				__HAL_RCC_GPIOA_CLK_ENABLE();
				__HAL_RCC_SPI1_CLK_ENABLE();

				GPIO_InitStruct.Pin       = SPI1_CLK ;
				GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
				GPIO_InitStruct.Pull      = GPIO_PULLUP;
				GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
				GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
				HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

				GPIO_InitStruct.Pin       = SPI1_MISO;
				GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
				HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

				GPIO_InitStruct.Pin       = SPI1_MOSI;
				GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
				HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

			#else
				__HAL_RCC_GPIOB_CLK_ENABLE();
				GPIO_InitStruct.Pin       = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
				GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
				GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
				GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
				GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
				HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

			#endif

			SPIHandle[SPI1_HANDLE_INDEX].Instance 				 = SPI1;

			switch(Prescalar)
			{
				case DIVIDE_2:
					SPIHandle[SPI1_HANDLE_INDEX].Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_2;
					break;
				case DIVIDE_4:
					SPIHandle[SPI1_HANDLE_INDEX].Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_4;
					break;
				case DIVIDE_8:
					SPIHandle[SPI1_HANDLE_INDEX].Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_8;
					break;
				case DIVIDE_16:
					SPIHandle[SPI1_HANDLE_INDEX].Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_16;
					break;
				case DIVIDE_32:
					SPIHandle[SPI1_HANDLE_INDEX].Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_32;
					break;
				case DIVIDE_64:
					SPIHandle[SPI1_HANDLE_INDEX].Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_64;
					break;
				case DIVIDE_128:
					SPIHandle[SPI1_HANDLE_INDEX].Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_128;
					break;
				case DIVIDE_256:
					SPIHandle[SPI1_HANDLE_INDEX].Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_256;
					break;
			}

			SPIHandle[SPI1_HANDLE_INDEX].Init.Direction 		 = SPI_DIRECTION_2LINES;
			SPIHandle[SPI1_HANDLE_INDEX].Init.CLKPhase 			 = SPI_PHASE_1EDGE;
			SPIHandle[SPI1_HANDLE_INDEX].Init.CLKPolarity 		 = SPI_POLARITY_LOW;
			SPIHandle[SPI1_HANDLE_INDEX].Init.DataSize 		 	 = SPI_DATASIZE_8BIT;
			SPIHandle[SPI1_HANDLE_INDEX].Init.FirstBit 		 	 = SPI_FIRSTBIT_MSB;
			SPIHandle[SPI1_HANDLE_INDEX].Init.TIMode 		  	 = SPI_TIMODE_DISABLE;
			SPIHandle[SPI1_HANDLE_INDEX].Init.CRCCalculation 	 = SPI_CRCCALCULATION_DISABLE;
			SPIHandle[SPI1_HANDLE_INDEX].Init.CRCPolynomial 	 = 7;
			SPIHandle[SPI1_HANDLE_INDEX].Init.CRCLength 		 = SPI_CRC_LENGTH_8BIT;
			SPIHandle[SPI1_HANDLE_INDEX].Init.NSS 				 = SPI_NSS_SOFT;
			SPIHandle[SPI1_HANDLE_INDEX].Init.NSSPMode 			 = SPI_NSS_PULSE_DISABLE;

			if (SPI_Mode == SPI_MASTER) {
				SPIHandle[SPI1_HANDLE_INDEX].Init.Mode = SPI_MODE_MASTER;
			} else {
				SPIHandle[SPI1_HANDLE_INDEX].Init.Mode = SPI_MODE_SLAVE;
			}

			HAL_SPI_Init(&SPIHandle[SPI1_HANDLE_INDEX]);
		 break;
	}
#endif
}


void SPI_Transmit(uint8_t *TxData,uint8_t *RxData,uint16_t Size)
{
	while(HAL_SPI_GetState(&SPIHandle[SPI1_HANDLE_INDEX]) != HAL_SPI_STATE_READY);
	HAL_SPI_TransmitReceive(&SPIHandle[SPI1_HANDLE_INDEX], (uint8_t*) TxData, RxData, Size, SPI_DATA_TIMEOUT);
}

uint8_t SPI_Receive()
{
	uint8_t Dummy = 0xFF,Data = 0;
	uint16_t Timeout = 1000;

	while((HAL_SPI_GetState(&SPIHandle[SPI1_HANDLE_INDEX]) != HAL_SPI_STATE_READY) && Timeout--);
	HAL_SPI_TransmitReceive(&SPIHandle[SPI1_HANDLE_INDEX],&Dummy,&Data,1,SPI_DATA_TIMEOUT);

	return Data;
}

uint8_t SPI_Transmit_Byte(uint8_t Data)
{
	uint8_t RxData;
	uint16_t Timeout = 1000;

	while((HAL_SPI_GetState(&SPIHandle[SPI1_HANDLE_INDEX]) != HAL_SPI_STATE_READY) && Timeout--);
	HAL_SPI_TransmitReceive(&SPIHandle[SPI1_HANDLE_INDEX],&Data,&RxData,1,SPI_DATA_TIMEOUT);

	return RxData;
}
