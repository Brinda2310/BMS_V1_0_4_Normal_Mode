/*
 * Power_Management.c
 *
 *  Created on: 10-Apr-2017
 *      Author: NIKHIL
 */
#include <Power_Management.h>

bool Sleep_Mode = false;

/* Variable which becomes true only when MCU wake up from sleep mode either from Vref pin or from external
 * switch press */
bool Wakeup_From_Sleep = false;

void MCU_Enter_Sleep_Mode()
{
#ifdef BMS_VERSION
	GPIO_InitTypeDef  GPIO_InitStruct;
	__HAL_RCC_PWR_CLK_ENABLE();

	RCC->AHB1SMENR = 0x0;
	RCC->AHB2SMENR = 0x0;
	RCC->AHB3SMENR = 0x0;

	RCC->APB1SMENR1 = 0x0;
	RCC->APB1SMENR2 = 0x0;
	RCC->APB2SMENR = 0x0;

	GPIO_InitStruct.Pin = MCU_WAKEUP_PIN;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Mode = WAKEUP_EDGE;

	/* Enable GPIOA clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	NVIC_SetPriority((IRQn_Type) (EXTI9_5_IRQn), 0x03);
	HAL_NVIC_EnableIRQ((IRQn_Type) (EXTI9_5_IRQn));

	/* Reduce the System clock to below 2 MHz */
	SystemClock_Decrease();

	HAL_SuspendTick();

	HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
#endif
}

void MCU_Exit_Sleep_Mode()
{
#ifdef BMS_VERSION
    HAL_PWREx_DisableLowPowerRunMode();
    /* Reinitialize all the peripherals as they were disabled before MCU going to sleep; But sleep mode holds
     * the global variables values to their previous state as before going to sleep */
	/* Configure the sysTick interrupt to 1mS(default) and Set the NVIC group priority to 4 */
	HAL_Init();

	/* Configure the system clock frequency (Peripherals clock) to 80MHz */
	Set_System_Clock_Frequency();

	/* Initialize the timer to 40mS and the same is used to achieve different loop rates */
	BMS_Timers_Init();

	/* Initialize the USART to 115200 baud rate to debug the code */
#if DEBUG_COM == ENABLE
	BMS_Debug_COM_Init();
#endif

	/* Initialize the status LEDs which indicates the SOC and SOH */
	BMS_Status_LEDs_Init();

	/* Configure the switch as input to wake up the BMS in case of sleep and same will be used
	 * to show the SOC and SOH on status LEDs*/
	BMS_Switch_Init();

	/* Configure the ISL94203 I2C communication to 100KHz */
	BMS_ASIC_Init();

#endif
}

void SystemClock_Decrease(void)
{
#if BMS_VERSION == BMS_V1
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* MSI is enabled after System reset, activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_0;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select MSI as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);

  /* Disable HSI to reduce power consumption since MSI is used from that point */
  __HAL_RCC_HSI_DISABLE();
#endif
}

void Set_System_Clock_Frequency(void)
{
#ifdef BMS_VERSION
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };

	/* MSI is enabled after System reset, activate PLL with MSI as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 40;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;

	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);

#endif
}

/* ISR which handles the wake up of MCU from sleep mode and resumes the operation */
void EXTI9_5_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(MCU_WAKEUP_PIN);
  /* This flag avoids the entering the sleep mode of MCU; Once it goes to sleep mode then wait for
   * external trigger to wake it up */
  if(Sleep_Mode == true)
  {
	  MCU_Exit_Sleep_Mode();
	  Wakeup_From_Sleep = true;
	  Sleep_Mode = false;
  }
}

