/*
 * IWDG_API.c
 *
 *  Created on: 25-Oct-2017
 *      Author: NIKHIL
 */

#include "IWDG_API.h"

#ifdef BMS_VERSION
	static IWDG_HandleTypeDef   IwdgHandle;
	TIM_HandleTypeDef           Input_Handle;
	RCC_ClkInitTypeDef          RCC_ClkInitStruct;
	RCC_OscInitTypeDef          RCC_OscInitStruct;

	uint16_t tmpCC4[2] = {0, 0};
	__IO uint32_t LSI_Frequency = 0;
	__IO uint32_t uwCaptureNumber = 0;
#endif

static uint32_t GetLSIFrequency(void)
{
	uint16_t Timeout = 2000;
#ifdef BMS_VERSION
	TIM_IC_InitTypeDef TIMInput_Config;

	RCC_OscInitTypeDef RCC_OscInitStruct;

	RCC_OscInitStruct.OscillatorType 	= RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.PLL.PLLState 		= RCC_PLL_NONE;
	RCC_OscInitStruct.LSIState 			= RCC_LSI_ON;

	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	__HAL_RCC_TIM16_CLK_ENABLE();

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
	HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 5, 0);

	HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

	Input_Handle.Instance = TIM16;

	Input_Handle.Init.Prescaler = 0;
	Input_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	Input_Handle.Init.Period = 0xFFFF;
	Input_Handle.Init.ClockDivision = 0;
	HAL_TIM_IC_Init(&Input_Handle);

	HAL_TIMEx_RemapConfig(&Input_Handle, TIM_TIM16_TI1_LSI);

	TIMInput_Config.ICPolarity = TIM_ICPOLARITY_RISING;
	TIMInput_Config.ICSelection = TIM_ICSELECTION_DIRECTTI;
	TIMInput_Config.ICPrescaler = TIM_ICPSC_DIV8;
	TIMInput_Config.ICFilter = 0;
	HAL_TIM_IC_ConfigChannel(&Input_Handle, &TIMInput_Config, TIM_CHANNEL_1);

	HAL_TIM_IC_Start_IT(&Input_Handle, TIM_CHANNEL_1);

	while (uwCaptureNumber != 2 && Timeout-- > 0)
	{
	}

	HAL_TIM_IC_Stop_IT(&Input_Handle, TIM_CHANNEL_1);

	HAL_TIM_IC_DeInit(&Input_Handle);

#endif
	return LSI_Frequency;
}

#ifdef BMS_VERSION
void TIM1_UP_TIM16_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&Input_Handle);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  uint32_t lsiperiod = 0;

  /* Get the Input Capture value */
  tmpCC4[uwCaptureNumber++] = HAL_TIM_ReadCapturedValue(&Input_Handle, TIM_CHANNEL_1);

  if (uwCaptureNumber >= 2)
  {
    /* Compute the period length */
    lsiperiod = (uint16_t)(0xFFFF - tmpCC4[0] + tmpCC4[1] + 1);

    /* Frequency computation */
    LSI_Frequency = (uint32_t) SystemCoreClock / lsiperiod;
    LSI_Frequency = LSI_Frequency * 8;
  }
}

#endif

uint8_t IWDG_Init(uint8_t Reset_Value)
{
	uint8_t Result = RESULT_OK;

#ifdef BMS_VERSION
	__HAL_RCC_CLEAR_RESET_FLAGS();

	LSI_Frequency = GetLSIFrequency();

	IwdgHandle.Instance 		= IWDG;
	IwdgHandle.Init.Prescaler 	= IWDG_PRESCALER_32;
	IwdgHandle.Init.Reload 		= ((LSI_Frequency * Reset_Value) / 32);
	IwdgHandle.Init.Window 		= IWDG_WINDOW_DISABLE;

	if (HAL_IWDG_Init(&IwdgHandle) != HAL_OK)
	{
		Result = RESULT_ERROR;
	}
	HAL_IWDG_Start(&IwdgHandle);

#endif
	return Result;
}

void IWDG_Reset_Counter(void)
{
#ifdef BMS_VERSION
	HAL_IWDG_Refresh(&IwdgHandle);
#endif
}

