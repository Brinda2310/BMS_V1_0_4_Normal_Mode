/*
 * BMS_Communication.c
 *
 *  Created on: 18-Jan-2017
 *      Author: NIKHIL
 */

#include <BMS_Serial_Communication.h>

bool Debug_COM_Enable = true;

/**
 * @brief  Function to initialize debug USART to 115200 baud rate
 * @param  None
 * @retval None
 */
void BMS_Debug_COM_Init()
{
	if(Debug_COM_Enable == true)
	{
		USART_Init(USART_1,SYSTEM_BUAD_RATE);
	}
}

/**
 * @brief  Function to receive data through debug COM port
 * @param  RxBuffer		: Pointer to the data buffer in which data is to be read
 * @param  Size			: Size of data buffer to be read over USART
 */
void BMS_Debug_COM_Read_Data(uint8_t *RxBuffer,uint16_t Size)
{
	if(Debug_COM_Enable == true)
	{
		USART_Read(USART_1,RxBuffer,Size);
	}
}

/*
 * @brief  Function to transmit the data over debug COM port
 * @param  TxBuffer		: Pointer to the data buffer to be sent over USART
 * @param  Size			: Size of data buffer to be sent over USART
 */
void BMS_Debug_COM_Write_Data(void *TxBuffer,uint16_t Size)
{
	if(Debug_COM_Enable == true)
	{
		USART_Write(USART_1,TxBuffer,Size);
		/* Delay is required to allow USART to send all the bytes from TxBuffer as it is configured in interrupt mode */
		Delay_Millis(4);
	}
}

