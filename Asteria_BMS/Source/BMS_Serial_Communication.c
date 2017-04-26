/*
 * BMS_Communication.c
 *
 *  Created on: 18-Jan-2017
 *      Author: NIKHIL
 */

#include <BMS_Serial_Communication.h>

void BMS_COM_Init()
{
	USART_Init(USART_1,SYSTEM_BUAD_RATE);
}

void BMS_COM_Read_Data(uint8_t *Data_Ptr,uint16_t Size)
{
	USART_Read(USART_1,Data_Ptr,Size);
}

void BMS_COM_Write_Data(void *Data_Ptr,uint16_t Size)
{
	USART_Write(USART_1,Data_Ptr,Size);
}

