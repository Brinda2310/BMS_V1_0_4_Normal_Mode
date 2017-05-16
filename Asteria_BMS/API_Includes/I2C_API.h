
/******************************************************************************
  * @file    I2C_API.h
  * @author  NIKHIL INGALE
  * @date    20-Jan-2017
  * @brief   This file contains all the functions prototypes for the I2C
  *          module driver
  ******************************************************************************/

#ifndef I2C_API_H_
#define I2C_API_H_

#include "Hardware_Config.h"

/* Number of I2C buses for stm32l432kc micro controller */
enum I2C_Number
{
	I2C_1 = 0,I2C_3
};

/* Function prototypes */
uint8_t I2C_Init(uint8_t I2C_Num,uint8_t I2C_Address,uint32_t Clock_Frequency);
uint8_t I2C_DeInit(uint8_t I2C_Num);
uint8_t I2C_WriteData(uint8_t I2C_Num,uint16_t Dev_Address,uint8_t *TxBuffer,uint16_t Size);
uint8_t I2C_ReadData(uint8_t I2C_Num,uint16_t Dev_Address,uint8_t *RxBuffer,uint8_t Size);

#endif /* I2C_API_H_ */
