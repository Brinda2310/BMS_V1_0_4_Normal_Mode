/*
 * BMS_Communication.c
 *
 *  Created on: 18-Jan-2017
 *      Author: NIKHIL
 */

#include <BMS_Serial_Communication.h>

void BMS_Debug_COM_Init()
{
#if DEBUG_COM == ENABLE
	USART_Init(USART_1,SYSTEM_BUAD_RATE);
#endif
}

void BMS_Debug_COM_Read_Data(uint8_t *Data_Ptr,uint16_t Size)
{
#if DEBUG_COM == ENABLE
	USART_Read(USART_1,Data_Ptr,Size);
#endif
}

void BMS_Debug_COM_Write_Data(void *Data_Ptr,uint16_t Size)
{
#if DEBUG_COM == ENABLE
	USART_Write(USART_1,Data_Ptr,Size);
#endif
}

