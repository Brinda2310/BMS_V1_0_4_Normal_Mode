
/******************************************************************************
  * @file    USART_API.h
  * @author  NIKHIL INGALE
  * @date    18-Jan-2017
  * @brief   This file contains all the functions prototypes for the USART
  *          module driver
  ******************************************************************************/
#ifndef USART_API_H_
#define USART_API_H_

#include "Hardware_Config.h"

/* Number of USARTs for stm32l432kc micro controller */
enum USART_number
{
	USART_1 = 0,USART_2
};

/* Function Prototypes */
uint8_t USART_Init(uint8_t USART_Num,int32_t Baud_Rate);
uint8_t USART_DeInit(uint8_t USART_Num);
uint8_t USART_Write(uint8_t USART_Num, void *TxBuffer,uint16_t Size);
uint8_t USART_Read(uint8_t USART_Num, void *RxBufffer,uint16_t Size);

#endif /* USART_API_H_ */
