/*
 * BMS_Watchdog.h
 *
 *  Created on: 25-Oct-2017
 *      Author: NIKHIL
 */

#ifndef BMS_WATCHDOG_H_
#define BMS_WATCHDOG_H_

#include <IWDG_API.h>

#define _2_SECONDS_WDG			2

extern bool BMS_Watchdog_Enable;

void BMS_watchdog_Init(void);
void BMS_Watchdog_Refresh(void);

#endif /* BMS_WATCHDOG_H_ */
