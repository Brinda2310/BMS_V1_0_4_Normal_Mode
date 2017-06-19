/*
 * AP_Communication.h
 *
 *  Created on: 02-May-2017
 *      Author: NIKHIL
 */

#ifndef AP_COMMUNICATION_H_
#define AP_COMMUNICATION_H_

#include "I2C_API.h"
#include "BMS_ASIC.h"
#include "RTC_API.h"

#define BMS_SMBUS_OWN_ADDRESS				0x52
#define BMS_SMBUS							I2C_3

#define MAX_AP_DATA_SIZE					20

/* Readable registers */
#define ALL_CELL_VOLTAGES_REG				0x00	/* Register for all cell voltages */
#define CELL1_VOLTAGE_REG					0x01
#define CELL2_VOLTAGE_REG					0x02
#define CELL3_VOLTAGE_REG					0x03
#define CELL4_VOLTAGE_REG					0x04
#define CELL5_VOLTAGE_REG					0x05
#define CELL6_VOLTAGE_REG					0x06
#define PACK_CURRENT_REG					0x07
#define PACK_VOLTAGE_REG					0x08
#define STATUS_FLAGS_REG					0x09
#define FLIGHT_STATUS_REG					0x0A

/*
 *0 0 0 0 0 0 Arm/DisARM Air/Ground
 * If ARMED/Air then do not go to sleep
 * If DisARMED and Ground then go to sleep
 */

/* Writable registers */
#define GPS_PACKET_REG						0x0A

enum AP_COM_Modes
{
	AP_COM_USART_MODE = 0, AP_COM_SMBUS_MODE,AP_COM_SPI_MODE
};


void AP_COM_Init(uint8_t Communication_Mode);
//static void BMS_Enable_Listen_Mode();
void Check_AP_Request();

#endif /* AP_COMMUNICATION_H_ */
