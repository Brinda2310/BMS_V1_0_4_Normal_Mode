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
#define MAX_SMBUS_REBOOT_COUNT				15		/* Load presence check time is 60 seconds */

/* Readable registers */
#define CELL1_VOLTAGE_REG					0x01
#define CELL2_VOLTAGE_REG					0x02
#define CELL3_VOLTAGE_REG					0x03
#define CELL4_VOLTAGE_REG					0x04
#define CELL5_VOLTAGE_REG					0x05
#define CELL6_VOLTAGE_REG					0x06
#define PACK_CURRENT_REG					0x07
#define PACK_VOLTAGE_REG					0x08
#define STATUS_FLAGS_REG					0x09

/* Writable registers */
#define GPS_PACKET_REG						0x0A
#define FLIGHT_STATUS_REG					0x0B
#define ALL_CELL_VOLTAGES_REG				0x0C	/* Register for all cell voltages */

#define GPS_DATE_TIME_DATA_SIZE				17		/* Data packet e.g. 0422062017,191234*/
#define FLIGHT_STATUS_DATA_SIZE				2		/* Data packet e.g. 0x0009 */

enum AP_COM_Modes
{
	AP_COM_USART_MODE = 0, AP_COM_SMBUS_MODE,AP_COM_SPI_MODE
};
/* ---------------------------------------------------------
 * DISARMED |	ARMED	|	AIR		|	GROUND	|	VALUE	|
 * ---------------------------------------------------------|
 * 	  0		|	 0		|	 0		|	  1		|	0x01	|
 * ---------------------------------------------------------|
 *    0     | 	 0		|	 1		|	  0		|	0x02	|
 * ---------------------------------------------------------|
 *    0		|	 1		|	 0		|	  0		|	0x04	|
 * ---------------------------------------------------------|
 *    1		|	 0		|	 0		|	  0		|	0x08	|
 * ---------------------------------------------------------|
 *    1		|	 0		|	 0		|	  1		|	0x09	|
 * ---------------------------------------------------------
 */
/* Flight mode statuses received from AP */
enum Flight_Status
{
	GROUND = 0x01, AIR = 0x02,ARMED = 0x04,DISARMED = 0x08,DISARMED_GROUND = 0x09
};

typedef union
{
	uint8_t bytes[32];
	float values[8];
}Pack_Info;

extern Pack_Info Pack_Data;
extern uint8_t GPS_Data[20];

/* Variable to decide whether sleep functionality in the code to be used or not;Decision is based
 * on data received from AP i.e. Flight_Status_Packet */
extern bool Sleep_Mode_Funtionality;

/* Function prototypes defined in the .c file */
uint8_t AP_COM_Init(uint8_t Communication_Mode);
uint8_t AP_COM_DeInit();
void BMS_Enable_Listen_Mode();
void BMS_Disable_Listen_Mode();

#endif /* AP_COMMUNICATION_H_ */
