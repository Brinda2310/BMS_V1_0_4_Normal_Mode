
#include <TIMER_API.h>

#ifdef BMS_VERSION
	/* Timer Structure defined in HAL layer and the same is used for handling the timer operations
	 * Number of PWM Timers for specific micro controller are defined in Hardware_Config.h file */
	TIM_HandleTypeDef  TimHandle;
	#if PWM_FUNCTION == ENABLE
		TIM_OC_InitTypeDef OC_Config;
		TIM_HandleTypeDef  PWMHandle[NUM_OF_PWM_TIMERS];
	#endif
#endif

/**
 * @brief  Initialize the Timer according to the specified parameters
 * @param  Timer_Num	: Timer Number of the specific micro controller [TIMER_1,TIMER_2 etc]
 * @param  Period		: Timer Period in milliseconds [20,40,100ms etc]
 * @retval RES_ERROR	: Not Successful
 * 		   RES_OK		: Successful
 */
uint8_t Timer_Init(uint8_t Timer_Num,uint32_t Period)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION

	switch(Timer_Num)
	{
	case TIMER_2:
		__HAL_RCC_TIM2_CLK_ENABLE();

	/* If TIMER2_INT_MODE macro is enabled then set the priority and enable the same.
	 * Timer priorities are defined in Hardware_Config.h file */
	#if TIMER2_INT_MODE == ENABLE
		#if (TIMER2_PRIORITY >=4 && TIMER2_PRIORITY <=7)
			HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
		#elif (TIMER2_PRIORITY >=8 && TIMER2_PRIORITY <=15)
			HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
		#else
			HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
		#endif
		HAL_NVIC_SetPriority(TIM2_IRQn,TIMER2_PRIORITY,TIMER2_SUBPRIORITY);
		HAL_NVIC_EnableIRQ(TIM2_IRQn);
	#endif
		TimHandle.Instance 	= TIM2;
		break;

	case TIMER_6:
		__HAL_RCC_TIM6_CLK_ENABLE();

	/* If TIMER6_INT_MODE macro is enabled then set the priority and enable the same.
	 * Timer priorities are defined in Hardware_Config.h file */
	#if TIMER6_INT_MODE == ENABLE
		#if (TIMER6_PRIORITY >=4 && TIMER6_PRIORITY <=7)
			HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
		#elif (TIMER6_PRIORITY >=8 && TIMER6_PRIORITY <=15)
			HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
		#else
			HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
		#endif
		HAL_NVIC_SetPriority(TIM6_DAC_IRQn,TIMER6_PRIORITY,TIMER6_SUBPRIORITY);
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
	#endif
		TimHandle.Instance 	= TIM6;
		break;

	case TIMER_7:
		__HAL_RCC_TIM7_CLK_ENABLE();

	/* If TIMER7_INT_MODE macro is enabled then set the priority and enable the same.
	 * Timer priorities are defined in Hardware_Config.h file */
	#if TIMER7_INT_MODE == ENABLE
		#if (TIMER7_PRIORITY >=4 && TIMER7_PRIORITY <=7)
			HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
		#elif (TIMER7_PRIORITY >=8 && TIMER7_PRIORITY <=15)
			HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
		#else
			HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
		#endif

			HAL_NVIC_SetPriority(TIM7_IRQn,TIMER7_PRIORITY,TIMER7_SUBPRIORITY);
			HAL_NVIC_EnableIRQ(TIM7_IRQn);
	#endif
		TimHandle.Instance 	= TIM7;
		break;

	}

	/* Configure the timer resolution value to 100uS. The same can be configured in Hardware_Config.h */
	TimHandle.Init.Prescaler         = _100uS_PRESCALAR;
	TimHandle.Init.Period            = ((Period * 10) - 1);
	TimHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TimHandle.Init.RepetitionCounter = 0;

	if(HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
		Result = RESULT_ERROR;

	/* if interrupts for timers are enabled then configure and enable them */
	#if TIMER2_INT_MODE == ENABLE
		if(HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
			Result = RESULT_ERROR;
	#else
		if(HAL_TIM_Base_Start(&TimHandle) != HAL_OK)
			Result = RESULT_ERROR;
	#endif

	#if TIMER6_INT_MODE == ENABLE
		if(HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
			Result = RESULT_ERROR;
	#else
		if(HAL_TIM_Base_Start(&TimHandle) != HAL_OK)
			Result = RESULT_ERROR;
	#endif


	#if TIMER7_INT_MODE == ENABLE
		if(HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
			Result = RESULT_ERROR;
	#else
		if(HAL_TIM_Base_Start(&TimHandle) != HAL_OK)
			Result = RESULT_ERROR;
	#endif

#endif
		return Result;
}

/**
 * @brief  Deinitialize the Timer according to the specified parameters(Diables the clock for timers based on parameter)
 * @param  Timer_Num	: Timer Number of the specific micro controller [TIMER_1,TIMER_2 etc]
 * @retval RES_ERROR	: Not Successful
 * 		   RES_OK		: Successful
 */
uint8_t Timer_DeInit(uint8_t Timer_Num)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION
	switch (Timer_Num)
	{
		case TIMER_1:
			 __HAL_RCC_TIM1_CLK_DISABLE();
			break;
		case TIMER_2:
			__HAL_RCC_TIM2_CLK_DISABLE();
			break;
		case TIMER_6:
			__HAL_RCC_TIM6_CLK_DISABLE();
			break;
		case TIMER_7:
			__HAL_RCC_TIM7_CLK_DISABLE();
			break;
		case TIMER_15:
			__HAL_RCC_TIM15_CLK_DISABLE();
			break;
		case TIMER_16:
			__HAL_RCC_TIM16_CLK_DISABLE();
			break;
		default:
		 	 Result = RESULT_ERROR;
			break;
	}
#endif
	return Result;
}


/* Timer interrrupt handlers for stm32l432kc micro controller */
#ifdef BMS_VERSION
#if TIMER2_INT_MODE == ENABLE
void TIM2_IRQHandler()
{
	if((TIM2->SR & TIM_FLAG_UPDATE) == TIM_FLAG_UPDATE)
	{
		if((TIM2->DIER & TIM_IT_UPDATE) == TIM_IT_UPDATE)
		{
			TIM2->SR = ~ TIM_IT_UPDATE;
			TIM2_PeriodElapsedCallback();
		}
	}
}
#endif

#if TIMER6_INT_MODE == ENABLE
void TIM6_DAC_IRQHandler()
{
	if((TIM6->SR & TIM_FLAG_UPDATE) == TIM_FLAG_UPDATE)
	{
		if((TIM6->DIER & TIM_IT_UPDATE) == TIM_IT_UPDATE)
		{
			TIM6->SR = ~ TIM_IT_UPDATE;
			TIM6_PeriodElapsedCallback();
		}
	}
}
#endif

#if TIMER7_INT_MODE == ENABLE
void TIM7_IRQHandler()
{
	if((TIM7->SR & TIM_FLAG_UPDATE) == TIM_FLAG_UPDATE)
	{
		if((TIM7->DIER & TIM_IT_UPDATE) == TIM_IT_UPDATE)
		{
			TIM7->SR = ~ TIM_IT_UPDATE;
			TIM7_PeriodElapsedCallback();
		}
	}
}
#endif

/* Call back functions for timer interrupt handlers. These are configured as __weak to avoid the errors
 * If use does not define these handlers in application files then these handlers will be called */
__weak void TIM2_PeriodElapsedCallback()
{
  UNUSED(0);
}

__weak void TIM6_PeriodElapsedCallback()
{
  UNUSED(0);
}

__weak void TIM7_PeriodElapsedCallback()
{
  UNUSED(0);
}

#endif

/**
 * @brief  Function to get the current timer/counter value and each tick value is equal to the resolution value
 * 		   used while configuring the timer
 * @param  Timer_Num	: Timer Number of the specific micro controller [TIMER_1,TIMER_2 etc]
 * @retval Returns the timer/counter value
 */
uint16_t Get_Current_Time_Count(uint8_t Timer_Num)
{
	uint16_t temp_time_value = 0;

#ifdef BMS_VERSION
	switch (Timer_Num)
	{
		case TIMER_2:
			temp_time_value = TIM2->CNT;
			break;
		case TIMER_6:
			temp_time_value = TIM6->CNT;
			break;
		case TIMER_7:
			temp_time_value = TIM7->CNT;
			break;
		default:
			temp_time_value = 0;
			break;
	}
#endif

	return temp_time_value;
}

/**
 * @brief  Initialize the PWM timer according to the specified parameters
 * @param  Timer_Num	: PWM Timer Number of the specific micro controller [TIMER_1,TIMER_2 etc]
 * @param  PWM_Period	: PWM_Timer Period in milliseconds [20,40,100ms etc]
 * @Note:  PWM functionality can used only when PWM_FUNCTION macro is enabled in Hardware_Config.h file
 * @retval RES_ERROR	: Not Successful
 * 		   RES_OK		: Successful
 */
uint8_t PWM_Init(uint8_t Timer_Num,uint32_t PWM_Period)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION
#if	PWM_FUNCTION == ENABLE
	GPIO_InitTypeDef   GPIO_InitStruct;
	switch (Timer_Num)
	{
		case TIMER_1:
			__HAL_RCC_TIM1_CLK_ENABLE();
			__HAL_RCC_GPIOA_CLK_ENABLE();

			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

		#if TIM1_CHANNEL1 == ENABLE
			GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
			GPIO_InitStruct.Pin = GPIO_PIN_8;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		#endif
		#if TIM1_CHANNEL2 == ENABLE
			GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
			GPIO_InitStruct.Pin = GPIO_PIN_9;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		#endif
		#if TIM1_CHANNEL3 == ENABLE
			GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
			GPIO_InitStruct.Pin = GPIO_PIN_10;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		#endif
		#if TIM1_CHANNEL4 == ENABLE
			GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
			GPIO_InitStruct.Pin = GPIO_PIN_11;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		#endif

			uint32_t PrescalerValue_1 = (uint32_t)(SystemCoreClock / TIM1_PWM_PRESCALAR) - 1;

			PWMHandle[TIM1_HANDLE_INDEX].Instance 			 	 = TIM1;
			PWMHandle[TIM1_HANDLE_INDEX].Init.Prescaler          = PrescalerValue_1;
			PWMHandle[TIM1_HANDLE_INDEX].Init.Period             = (PWM_Period-1);
			PWMHandle[TIM1_HANDLE_INDEX].Init.ClockDivision      = 0;
			PWMHandle[TIM1_HANDLE_INDEX].Init.CounterMode        = TIM_COUNTERMODE_UP;
			PWMHandle[TIM1_HANDLE_INDEX].Init.RepetitionCounter  = 0;

			if(HAL_TIM_PWM_Init(&PWMHandle[TIM1_HANDLE_INDEX]) != HAL_OK)
				Result = RESULT_ERROR;

			OC_Config.OCMode       = TIM_OCMODE_PWM1;
			OC_Config.OCPolarity   = TIM_OCPOLARITY_HIGH;
			OC_Config.OCFastMode   = TIM_OCFAST_DISABLE;
			OC_Config.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
			OC_Config.OCNIdleState = TIM_OCNIDLESTATE_RESET;
			OC_Config.OCIdleState  = TIM_OCIDLESTATE_RESET;
			OC_Config.Pulse 	   = 0;

		#if TIM1_CHANNEL1 == ENABLE
			if(HAL_TIM_PWM_ConfigChannel(&PWMHandle[TIM1_HANDLE_INDEX], &OC_Config, TIM_CHANNEL_1) != HAL_OK)
				Result = RESULT_ERROR;
			if(HAL_TIM_PWM_Start(&PWMHandle[TIM1_HANDLE_INDEX], TIM_CHANNEL_1) != HAL_OK)
				Result = RESULT_ERROR;
		#endif
		#if TIM1_CHANNEL2 == ENABLE
			if(HAL_TIM_PWM_ConfigChannel(&PWMHandle[TIM1_HANDLE_INDEX], &OC_Config, TIM_CHANNEL_2) != HAL_OK)
				Result = RESULT_ERROR;
			if(HAL_TIM_PWM_Start(&PWMHandle[TIM1_HANDLE_INDEX], TIM_CHANNEL_2)!= HAL_OK)
				Result = RESULT_ERROR;
		#endif
		#if TIM1_CHANNEL3 == ENABLE
			if(HAL_TIM_PWM_ConfigChannel(&PWMHandle[TIM1_HANDLE_INDEX], &OC_Config, TIM_CHANNEL_3) != HAL_OK)
				Result = RESULT_ERROR;
			if(HAL_TIM_PWM_Start(&PWMHandle[TIM1_HANDLE_INDEX], TIM_CHANNEL_3) != HAL_OK)
			Result = RESULT_ERROR;
		#endif
		#if TIM1_CHANNEL4 == ENABLE
			if(	HAL_TIM_PWM_ConfigChannel(&PWMHandle[TIM1_HANDLE_INDEX], &OC_Config, TIM_CHANNEL_4) != HAL_OK)
				Result = RESULT_ERROR;
			if(HAL_TIM_PWM_Start(&PWMHandle[TIM1_HANDLE_INDEX], TIM_CHANNEL_4) != HAL_OK)
				Result = RESULT_ERROR;
		#endif

			break;

		case TIMER_2:
			__HAL_RCC_TIM2_CLK_ENABLE();
			__HAL_RCC_GPIOA_CLK_ENABLE();

			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

			GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
		#if TIM2_CHANNEL1 == ENABLE
			#if TIM2_CHANNEL1_REMAP == ENABLE
				GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
				GPIO_InitStruct.Pin = GPIO_PIN_5;
				HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
			#else
				GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
				GPIO_InitStruct.Pin = GPIO_PIN_0;
				HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
			#endif
		#endif
		#if TIM2_CHANNEL2 == ENABLE
			#if TIM2_CHANNEL1_REMAP == ENABLE
				__HAL_RCC_GPIOB_CLK_ENABLE();
				GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
				GPIO_InitStruct.Pin = GPIO_PIN_3;
				HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
			#else
				GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
				GPIO_InitStruct.Pin = GPIO_PIN_1;
				HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
			#endif
		#endif
		#if TIM2_CHANNEL3 == ENABLE
			GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
			GPIO_InitStruct.Pin = GPIO_PIN_2;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		#endif
		#if TIM2_CHANNEL4 == ENABLE
			GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
			GPIO_InitStruct.Pin = GPIO_PIN_3;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		#endif

			uint32_t PrescalerValue_2 = (uint32_t)(SystemCoreClock / TIM2_PWM_PRESCALAR) - 1;

			PWMHandle[TIM2_HANDLE_INDEX].Instance 			 	 = TIM2;
			PWMHandle[TIM2_HANDLE_INDEX].Init.Prescaler          = PrescalerValue_2;
			PWMHandle[TIM2_HANDLE_INDEX].Init.Period             = (PWM_Period-1);
			PWMHandle[TIM2_HANDLE_INDEX].Init.ClockDivision      = 0;
			PWMHandle[TIM2_HANDLE_INDEX].Init.CounterMode        = TIM_COUNTERMODE_UP;
			PWMHandle[TIM2_HANDLE_INDEX].Init.RepetitionCounter  = 0;

			HAL_TIM_PWM_Init(&PWMHandle[TIM2_HANDLE_INDEX]);

			OC_Config.OCMode       = TIM_OCMODE_PWM1;
			OC_Config.OCPolarity   = TIM_OCPOLARITY_HIGH;
			OC_Config.OCFastMode   = TIM_OCFAST_DISABLE;
			OC_Config.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
			OC_Config.OCNIdleState = TIM_OCNIDLESTATE_RESET;
			OC_Config.OCIdleState  = TIM_OCIDLESTATE_RESET;
			OC_Config.Pulse 	   = 0;

		#if TIM2_CHANNEL1 == ENABLE
			if(HAL_TIM_PWM_ConfigChannel(&PWMHandle[TIM2_HANDLE_INDEX], &OC_Config, TIM_CHANNEL_1) != HAL_OK)
				Result = RESULT_ERROR;
			if(HAL_TIM_PWM_Start(&PWMHandle[TIM2_HANDLE_INDEX], TIM_CHANNEL_1) != HAL_OK)
				Result = RESULT_ERROR;
		#endif
		#if TIM2_CHANNEL2 == ENABLE
			if(HAL_TIM_PWM_ConfigChannel(&PWMHandle[TIM2_HANDLE_INDEX], &OC_Config, TIM_CHANNEL_2) != HAL_OK)
				Result = RESULT_ERROR;
			if(HAL_TIM_PWM_Start(&PWMHandle[TIM2_HANDLE_INDEX], TIM_CHANNEL_2) != HAL_OK)
				Result = RESULT_ERROR;
		#endif
		#if TIM2_CHANNEL3 == ENABLE
			if(HAL_TIM_PWM_ConfigChannel(&PWMHandle[TIM2_HANDLE_INDEX], &OC_Config, TIM_CHANNEL_3) != HAL_OK)
				Result = RESULT_ERROR;
			if(HAL_TIM_PWM_Start(&PWMHandle[TIM2_HANDLE_INDEX], TIM_CHANNEL_3) != HAL_OK)
				Result = RESULT_ERROR;
		#endif
		#if TIM2_CHANNEL4 == ENABLE
			if(HAL_TIM_PWM_ConfigChannel(&PWMHandle[TIM2_HANDLE_INDEX], &OC_Config, TIM_CHANNEL_4) != HAL_OK)
				Result = RESULT_ERROR;
			if(HAL_TIM_PWM_Start(&PWMHandle[TIM2_HANDLE_INDEX], TIM_CHANNEL_4) != HAL_OK)
				Result = RESULT_ERROR;
		#endif

			break;

		case TIMER_15:
			__HAL_RCC_TIM15_CLK_ENABLE();
			__HAL_RCC_GPIOA_CLK_ENABLE();

			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

		#if TIM15_CHANNEL1 == ENABLE
			GPIO_InitStruct.Alternate = GPIO_AF14_TIM15;
			GPIO_InitStruct.Pin = GPIO_PIN_2;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		#endif
		#if TIM15_CHANNEL2 == ENABLE
			GPIO_InitStruct.Alternate = GPIO_AF14_TIM15;
			GPIO_InitStruct.Pin = GPIO_PIN_3;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		#endif

			uint32_t PrescalerValue_3 = (uint32_t)(SystemCoreClock / TIM2_PWM_PRESCALAR) - 1;

			PWMHandle[TIM15_HANDLE_INDEX].Instance 			 	 = TIM15;
			PWMHandle[TIM15_HANDLE_INDEX].Init.Prescaler          = PrescalerValue_3;
			PWMHandle[TIM15_HANDLE_INDEX].Init.Period             = (PWM_Period-1);
			PWMHandle[TIM15_HANDLE_INDEX].Init.ClockDivision      = 0;
			PWMHandle[TIM15_HANDLE_INDEX].Init.CounterMode        = TIM_COUNTERMODE_UP;
			PWMHandle[TIM15_HANDLE_INDEX].Init.RepetitionCounter  = 0;

			HAL_TIM_PWM_Init(&PWMHandle[TIM15_HANDLE_INDEX]);

			OC_Config.OCMode       = TIM_OCMODE_PWM1;
			OC_Config.OCPolarity   = TIM_OCPOLARITY_HIGH;
			OC_Config.OCFastMode   = TIM_OCFAST_DISABLE;
			OC_Config.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
			OC_Config.OCNIdleState = TIM_OCNIDLESTATE_RESET;
			OC_Config.OCIdleState  = TIM_OCIDLESTATE_RESET;
			OC_Config.Pulse 	   = 0;

		#if TIM15_CHANNEL1 == ENABLE
			if(HAL_TIM_PWM_ConfigChannel(&PWMHandle[TIM15_HANDLE_INDEX], &OC_Config, TIM_CHANNEL_1) != HAL_OK)
				Result = RESULT_ERROR;
			if(HAL_TIM_PWM_Start(&PWMHandle[TIM15_HANDLE_INDEX], TIM_CHANNEL_1) != HAL_OK)
				Result = RESULT_ERROR;
		#endif
		#if TIM15_CHANNEL2 == ENABLE
			if(HAL_TIM_PWM_ConfigChannel(&PWMHandle[TIM15_HANDLE_INDEX], &OC_Config, TIM_CHANNEL_2) != HAL_OK)
				Result = RESULT_ERROR;
			if(HAL_TIM_PWM_Start(&PWMHandle[TIM15_HANDLE_INDEX], TIM_CHANNEL_2) != HAL_OK)
				Result = RESULT_ERROR;
		#endif

			break;

		case TIMER_16:
			__HAL_RCC_TIM16_CLK_ENABLE();
			__HAL_RCC_GPIOA_CLK_ENABLE();

			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

		#if TIM16_CHANNEL1 == ENABLE
			GPIO_InitStruct.Alternate = GPIO_AF14_TIM16;
			GPIO_InitStruct.Pin = GPIO_PIN_6;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		#endif

			uint32_t PrescalerValue_4 = (uint32_t)(SystemCoreClock / TIM2_PWM_PRESCALAR) - 1;

			PWMHandle[TIM16_HANDLE_INDEX].Instance 			 	 = TIM16;
			PWMHandle[TIM16_HANDLE_INDEX].Init.Prescaler          = PrescalerValue_4;
			PWMHandle[TIM16_HANDLE_INDEX].Init.Period             = (PWM_Period-1);
			PWMHandle[TIM16_HANDLE_INDEX].Init.ClockDivision      = 0;
			PWMHandle[TIM16_HANDLE_INDEX].Init.CounterMode        = TIM_COUNTERMODE_UP;
			PWMHandle[TIM16_HANDLE_INDEX].Init.RepetitionCounter  = 0;

			HAL_TIM_PWM_Init(&PWMHandle[TIM16_HANDLE_INDEX]);

			OC_Config.OCMode       = TIM_OCMODE_PWM1;
			OC_Config.OCPolarity   = TIM_OCPOLARITY_HIGH;
			OC_Config.OCFastMode   = TIM_OCFAST_DISABLE;
			OC_Config.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
			OC_Config.OCNIdleState = TIM_OCNIDLESTATE_RESET;
			OC_Config.OCIdleState  = TIM_OCIDLESTATE_RESET;
			OC_Config.Pulse 	   = 0;

		#if TIM16_CHANNEL1 == ENABLE
			if(HAL_TIM_PWM_ConfigChannel(&PWMHandle[TIM16_HANDLE_INDEX], &OC_Config, TIM_CHANNEL_1) != HAL_OK)
				Result = RESULT_ERROR;
			if(HAL_TIM_PWM_Start(&PWMHandle[TIM16_HANDLE_INDEX], TIM_CHANNEL_1) != HAL_OK)
				Result = RESULT_ERROR;
		#endif

			break;
		default:
			break;
	}
#endif
#endif
return Result;
}

/**
 * @brief  Generate the PWM output on specified channel number
 * @param  Timer_Num	: Timer Number of the specific micro controller [TIMER_1,TIMER_2 etc]
 * @param  PWM_Value	: PWM ON period in microseconds [1000,2000us etc]
 * @retval RES_ERROR	: Not Successful
 * 		   RES_OK		: Successful
 */
uint8_t Set_PWM_value(uint8_t PWM_Channel_Num,uint16_t PWM_Value)
{
	uint8_t Result = RESULT_OK;
#ifdef BMS_VERSION

#if PWM_FUNCTION == ENABLE
	switch(PWM_Channel_Num)
	{
		case TIM1_CHANNEL_1:
			__HAL_TIM_SET_COMPARE(&PWMHandle[TIM1_HANDLE_INDEX],TIM_CHANNEL_1,PWM_Value);
			break;
		case TIM1_CHANNEL_2:
			__HAL_TIM_SET_COMPARE(&PWMHandle[TIM1_HANDLE_INDEX],TIM_CHANNEL_2,PWM_Value);
			break;
		case TIM1_CHANNEL_3:
			__HAL_TIM_SET_COMPARE(&PWMHandle[TIM1_HANDLE_INDEX],TIM_CHANNEL_3,PWM_Value);
			break;
		case TIM1_CHANNEL_4:
			__HAL_TIM_SET_COMPARE(&PWMHandle[TIM1_HANDLE_INDEX],TIM_CHANNEL_4,PWM_Value);
			break;

		case TIM2_CHANNEL_1:
			__HAL_TIM_SET_COMPARE(&PWMHandle[TIM2_HANDLE_INDEX],TIM_CHANNEL_1,PWM_Value);
			break;
		case TIM2_CHANNEL_2:
			__HAL_TIM_SET_COMPARE(&PWMHandle[TIM2_HANDLE_INDEX],TIM_CHANNEL_2,PWM_Value);
			break;
		case TIM2_CHANNEL_3:
			__HAL_TIM_SET_COMPARE(&PWMHandle[TIM2_HANDLE_INDEX],TIM_CHANNEL_3,PWM_Value);
			break;
		case TIM2_CHANNEL_4:
			__HAL_TIM_SET_COMPARE(&PWMHandle[TIM2_HANDLE_INDEX],TIM_CHANNEL_4,PWM_Value);
			break;

		case TIM15_CHANNEL_1:
			__HAL_TIM_SET_COMPARE(&PWMHandle[TIM15_HANDLE_INDEX],TIM_CHANNEL_1,PWM_Value);
			break;
		case TIM15_CHANNEL_2:
			__HAL_TIM_SET_COMPARE(&PWMHandle[TIM15_HANDLE_INDEX],TIM_CHANNEL_2,PWM_Value);
			break;

		case TIM16_CHANNEL_1:
			__HAL_TIM_SET_COMPARE(&PWMHandle[TIM16_HANDLE_INDEX],TIM_CHANNEL_1,PWM_Value);
			break;
		default :
			Result = RESULT_ERROR;
			break;
	}
#endif
#endif
	return Result;
}

/**
 * @brief  Generates the delay in milliseconds with value specified in paramter
 * @param  Delay_Value	: Delay value in milliseconds (10,100,1000ms)
 * @retval None
 */
void Delay_Millis(uint32_t Delay_Value)
{
#ifdef BMS_VERSION
	HAL_Delay(Delay_Value);
#endif
}

