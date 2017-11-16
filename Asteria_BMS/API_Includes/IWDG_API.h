/*
 * IWDG_API.h
 *
 *  Created on: 25-Oct-2017
 *      Author: NIKHIL
 */

#ifndef IWDG_API_H_
#define IWDG_API_H_

#include "Hardware_Config.h"

uint8_t IWDG_Init(uint8_t Reset_Value);
void IWDG_Reset_Counter(void);

#endif /* IWDG_API_H_ */
