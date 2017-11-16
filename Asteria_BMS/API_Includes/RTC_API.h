/*
 * RTC_API.h
 *
 *  Created on: 09-Feb-2017
 *      Author: NIKHIL
 */

#ifndef RTC_API_H_
#define RTC_API_H_


#include "Hardware_Config.h"

#ifdef BMS_VERSION
	extern RTC_HandleTypeDef RtcHandle;

#define RTC_CLOCK_SOURCE_LSE
/*#define RTC_CLOCK_SOURCE_LSI*/

#ifdef RTC_CLOCK_SOURCE_LSI
#define RTC_ASYNCH_PREDIV    0x7F
#define RTC_SYNCH_PREDIV     0xF9
#endif

#ifdef RTC_CLOCK_SOURCE_LSE
#define RTC_ASYNCH_PREDIV  	0x7F
#define RTC_SYNCH_PREDIV  	0x00FF
#endif

#endif

typedef struct
{
	uint8_t Day;
	uint8_t Date;
	uint8_t Month;
	uint8_t Year;
	uint8_t Hours;
	uint8_t Minutes;
	uint8_t Seconds;
} RTC_Data;

extern RTC_Data RTC_Info;

enum WeekDays
{
	MONDAY = 1, TUESDAY,WEDNESDAY,THURSDAY,FRIDAY,SATURDAY,SUNDAY
};

enum Months
{
	JANUARY = 1,FEBRUARY,MARCH,APRIL,MAY,JUNE,JULY,AUGUST,SEPTEMBER,OCTOBER = 16,NOVEMBER,DECEMBER
};

uint8_t RTC_Init(void);
uint8_t RTC_Set_Date(uint8_t *Day, uint8_t *Date,uint8_t *Month,uint8_t *Year);
uint8_t RTC_Get_Date(uint8_t *Day,uint8_t *Date,uint8_t *Month, uint8_t *Year);
uint8_t RTC_Set_Time(uint8_t *Hours,uint8_t *Minutes,uint8_t *Seconds);
uint8_t RTC_Get_Time(uint8_t *Hours,uint8_t *Minutes,uint8_t *Seconds);
uint8_t RTC_AlarmConfig(uint8_t *Day,uint8_t * Hours,uint8_t * Minutes, uint8_t *Seconds);
uint8_t RTC_TimeShow(uint8_t* showtime);

#endif /* RTC_API_H_ */
