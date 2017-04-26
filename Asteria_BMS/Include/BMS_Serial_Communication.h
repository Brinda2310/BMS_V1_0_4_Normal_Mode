/*
 * BMS_Communication.h
 *
 *  Created on: 18-Jan-2017
 *      Author: NIKHIL
 */

#ifndef BMS_SERIAL_COMMUNICATION_H_
#define BMS_SERIAL_COMMUNICATION_H_

#include "USART_API.h"

#define SYSTEM_BUAD_RATE							115200

void BMS_COM_Init(void);
void BMS_COM_Read_Data(uint8_t *Data_Ptr,uint16_t Size);
void BMS_COM_Write_Data(void *Data_Ptr,uint16_t Size);

#endif /* BMS_SERIAL_COMMUNICATION_H_ */
