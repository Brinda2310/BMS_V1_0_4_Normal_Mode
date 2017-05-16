/*
 * BMS_Communication.c
 *
 *  Created on: 18-Jan-2017
 *      Author: NIKHIL
 */

#include <BMS_Serial_Communication.h>

/* Function to initialize debug USART to 115200 baud rate */
void BMS_Debug_COM_Init()
{
#if DEBUG_COM == ENABLE
	USART_Init(USART_1,SYSTEM_BUAD_RATE);
#endif
}

/**
 * @brief  Function to receive data through debug COM port
 * @param  RxBuffer		: Pointer to the data buffer in which data is read
 * @param  Size			: Size of data buffer to be read through COM port
 */
void BMS_Debug_COM_Read_Data(uint8_t *RxBuffer,uint16_t Size)
{
#if DEBUG_COM == ENABLE
	USART_Read(USART_1,RxBuffer,Size);
#endif
}

/*
 * @brief  Function to transmit the data over debug COM port
 * @param  TxBuffer		: Pointer to the data buffer to be sent over COM port
 * @param  Size			: Size of data buffer to be sent over COM port
 */
void BMS_Debug_COM_Write_Data(void *TxBuffer,uint16_t Size)
{
#if DEBUG_COM == ENABLE
	USART_Write(USART_1,TxBuffer,Size);
#endif
}

