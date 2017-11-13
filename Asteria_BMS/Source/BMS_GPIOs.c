/*
 * BMS_GPIOs.c
 *
 *  Created on: 26-Apr-2017
 *      Author: NIKHIL
 */

#include <BMS_GPIOs.h>
#include <BMS_Serial_Communication.h>
#include <BMS_ASIC.h>

/**
 * @brief  Function to initialize the switch functionality connected to respective GPIO of MCU
 * @param  None
 * @retval None
 */
void BMS_Switch_Init()
{
	GPIO_Init(BMS_SWITCH_PORT,BMS_SWITCH,GPIO_INPUT,NOPULL);
}

/**
 * @brief  Function to read the switch status. Switch is pulled up with internal resistor,
 * 		   If pressed pin status will be LOW and if not pressed it will remain to high state
 * @param  None
 * @retval PRESSED		: Switch is pressed
 * 		   NOT_PRESSED	: Switch is not pressed
 */
uint8_t BMS_Read_Switch_Status()
{
	if(GPIO_Read(BMS_SWITCH_PORT,BMS_SWITCH) == PIN_HIGH)
	{
		return PRESSED;
	}
	else
	{
		return NOT_PRESSED;
	}
}

/**
 * @brief  Function to initialize the LEDs connected to respective GPIOs of MCU
 * @param  None
 * @retval None
 */
void BMS_Status_LEDs_Init()
{
#if DEBUG_COM == DISABLE
	GPIO_Init(LED1_PORT,LED_1,GPIO_OUTPUT,NOPULL);
	GPIO_Init(LED2_PORT,LED_2,GPIO_OUTPUT,NOPULL);
	GPIO_Init(LED3_PORT,LED_3,GPIO_OUTPUT,NOPULL);
	GPIO_Init(LED4_PORT,LED_4,GPIO_OUTPUT,NOPULL);

	GPIO_Write(LED1_PORT,LED_1,PIN_HIGH);
	GPIO_Write(LED2_PORT,LED_2,PIN_HIGH);
	GPIO_Write(LED3_PORT,LED_3,PIN_HIGH);
	GPIO_Write(LED4_PORT,LED_4,PIN_HIGH);
#endif

#if BOARD_STATUS_ERROR_LED == ENABLE
	GPIO_Init(LED5_PORT,LED_5,GPIO_OUTPUT,NOPULL);
	GPIO_Init(LED6_PORT,LED_6,GPIO_OUTPUT,NOPULL);

	GPIO_Write(LED5_PORT,LED_5,PIN_HIGH);
	GPIO_Write(LED6_PORT,LED_6,PIN_HIGH);
#endif

}

/**
 * @brief  Function to toggle the status LEDs connected on board; Used only for debugging
 * @param  None
 * @retval None
 */
void BMS_Status_LED_Toggle()
{
#if DEBUG_COM == DISABLE
	GPIO_Write(LED1_PORT,LED_1,PIN_TOGGLE);
	GPIO_Write(LED2_PORT,LED_2,PIN_TOGGLE);
	GPIO_Write(LED3_PORT,LED_3,PIN_TOGGLE);
	GPIO_Write(LED4_PORT,LED_4,PIN_TOGGLE);
#endif
	GPIO_Write(LED6_PORT,LED_6,PIN_TOGGLE);
}

/**
 * @brief  Function to show the LED pattern upon switch press; It shows SOC and SOH based
 * 		   on time for which switch is pressed
 * @param  Pattern_Type		: Type of pattern to be shown on LEDs (SOC/SOH)
 * @param  Status			: Type of status (HIDE_STATUS/SHOW_STATUS)
 * @retval None
 */
void BMS_Show_LED_Pattern(uint8_t Pattern_Type,uint8_t Status)
{
	float Battery_Health = 0.0;

	if(Pattern_Type == SOC)
	{
		Battery_Health = Get_BMS_Capacity_Remaining();
		if(Battery_Health > 0.00f && Battery_Health <= 25.00f)
		{
			GPIO_Write(LED1_PORT,LED_1,PIN_LOW);
		}
		else if (Battery_Health > 25.00f && Battery_Health <= 50.00f)
		{
			GPIO_Write(LED1_PORT,LED_1,PIN_LOW);
#if DEBUG_COM == DISABLE
			GPIO_Write(LED2_PORT,LED_2,PIN_LOW);
#endif
		}
		else if (Battery_Health > 50.00f && Battery_Health <= 75.00f)
		{
			GPIO_Write(LED1_PORT,LED_1,PIN_LOW);
#if DEBUG_COM == DISABLE
			GPIO_Write(LED2_PORT,LED_2,PIN_LOW);
			GPIO_Write(LED3_PORT,LED_3,PIN_LOW);
#endif
		}
		else
		{
#if DEBUG_COM == DISABLE
			GPIO_Write(LED1_PORT, LED_1, PIN_LOW);
			GPIO_Write(LED2_PORT, LED_2, PIN_LOW);
			GPIO_Write(LED3_PORT, LED_3, PIN_LOW);
			GPIO_Write(LED4_PORT, LED_4, PIN_LOW);
#endif
		}
	}
	else if (Pattern_Type == SOH)
	{
//		BMS_Debug_COM_Write_Data("Long Press\r",11);
	}

	if(Status == HIDE_STATUS)
	{
#if DEBUG_COM == DISABLE
		GPIO_Write(LED1_PORT, LED_1, PIN_HIGH);
		GPIO_Write(LED2_PORT, LED_2, PIN_HIGH);
		GPIO_Write(LED3_PORT, LED_3, PIN_HIGH);
		GPIO_Write(LED4_PORT, LED_4, PIN_HIGH);
#endif
	}
}
