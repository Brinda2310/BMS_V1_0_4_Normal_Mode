/*
 * SPI_API.h
 *
 *  Created on: 25-Jan-2017
 *      Author: NIKHIL
 */

#ifndef SPI_API_H_
#define SPI_API_H_

#include "Hardware_Config.h"

#ifdef BMS_VERSION
	extern SPI_HandleTypeDef SPIHandle[NUM_OF_SPI_BUSES];
#endif

enum
{
	TRANSFER_WAIT,
	TRANSFER_COMPLETE,
	TRANSFER_ERROR
};
enum SPI_Num
{
	SPI_1 = 0, SPI_3,SPI_MASTER,SPI_SLAVE
};

enum SPI_Prescalars
{
	DIVIDE_2 = 0,DIVIDE_4,DIVIDE_8,DIVIDE_16,DIVIDE_32,DIVIDE_64,DIVIDE_128,DIVIDE_256
};

void SPI_Init(uint8_t SPI_Num,uint8_t Prescalar,uint8_t SPI_Mode);
void SPI_Transmit(uint8_t *TxData,uint8_t *RxData,uint16_t Size);
uint8_t SPI_Transmit_Byte(uint8_t Data);

#endif /* SPI_API_H_ */
