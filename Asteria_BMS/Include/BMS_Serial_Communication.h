/*
 * BMS_Serial_Communication.h
 *
 *  Created on: 18-Jan-2017
 *      Author: NIKHIL
 */

#ifndef BMS_SERIAL_COMMUNICATION_H_
#define BMS_SERIAL_COMMUNICATION_H_

#include <USART_API.h>

#define SYSTEM_BUAD_RATE							115200

/* Prototypes for the function defined in BMS_Serial_Communication.c file */
void BMS_Debug_COM_Init(void);
void BMS_Debug_COM_Read_Data(uint8_t *RxBuffer,uint16_t Size);
void BMS_Debug_COM_Write_Data(void *TxBuffer,uint16_t Size);

#endif /* BMS_SERIAL_COMMUNICATION_H_ */
