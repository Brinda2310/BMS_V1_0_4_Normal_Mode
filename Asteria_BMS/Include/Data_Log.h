/*
 * Data_Log.h
 *
 *  Created on: 16-Feb-2017
 *      Author: NIKHIL
 */

#ifndef DATA_LOG_H_
#define DATA_LOG_H_

#include "TIMER_API.h"
#include "ff.h"
#include "GPIO_API.h"
#include "BMS_Communication.h"
#include "FLASH_API.h"

#define DIRECTORY_NAME_START_ADDRESS				ADDR_FLASH_PAGE_127
#define DIRECTORY_NAME_END_ADDRESS					ADDR_FLASH_PAGE_127+8

#define THOUSANDS_DIGIT_POS							6
#define HUNDREDS_DIGIT_POS							(THOUSANDS_DIGIT_POS + 1)
#define TENS_DIGIT_POS								(HUNDREDS_DIGIT_POS + 1)
#define UNITS_DIGIT_POS								(TENS_DIGIT_POS + 1)
#define DIR_COUNT_TERMINATOR						0x09
#define FILE_COUNT_TERMINATOR						0x0A

uint8_t Create_Log_File(uint8_t *File_Name);
uint8_t Create_Log_Summary_File();
uint8_t Update_Log_Summary_File();
uint8_t Create_Directory();

//void Write_Data_To_File(uint8_t *Data);
//static void Maintain_Directory();

static uint8_t Directory_Name[] = "_1/";
static uint8_t File_Name[] = "Log_16_02_2017.txt";

#endif /* DATA_LOG_H_ */
