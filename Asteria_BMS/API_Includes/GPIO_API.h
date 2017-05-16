
/******************************************************************************
  * @file    GPIO_API.h
  * @author  NIKHIL INGALE
  * @date    27-Jan-2017
  * @brief   This file contains all the functions prototypes for the GPIO
  *          module driver
  ******************************************************************************/

#ifndef GPIO_API_H_
#define GPIO_API_H_

#include "Hardware_Config.h"

/* Number of GPIO ports for stm32l432kc micro controller */
enum GPIO_PORTS
{
	GPIO_A = 0,GPIO_B
};
enum GPIO_Modes
{
	GPIO_INPUT = 0,GPIO_OUTPUT ,GPIO_FLOATING
};
enum GPIO_States
{
	PIN_LOW = 0,PIN_HIGH,PIN_TOGGLE
};

enum GPIO_Init_States
{
	PULLUP = 0,PULLDOWN,NOPULL
};

/* Function Prototypes */
uint8_t GPIO_Init(uint8_t GPIO_Port,uint32_t GPIO_Pin,uint8_t Mode,uint8_t GPIO_Init_State);
uint8_t GPIO_DeInit(uint8_t GPIO_Port,uint32_t GPIO_Pin);
uint8_t GPIO_Write(uint8_t GPIO_Port,uint32_t GPIO_Pin,uint8_t GPIO_State);
uint8_t GPIO_Read(uint8_t GPIO_Port,uint32_t GPIO_Pin);

#endif /* GPIO_API_H_ */
