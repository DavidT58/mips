/**
 ******************************************************************************
 * File Name          : TIM.c
 * Description        : This file provides code for the configuration
 *                      of the TIM instances.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

TIM_HandleTypeDef htim1;

/* TIM1 init function */
void MX_TIM1_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig =
	{ 0 };
	TIM_MasterConfigTypeDef sMasterConfig =
	{ 0 };
	TIM_IC_InitTypeDef sConfigIC =
	{ 0 };

	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 0;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 65535;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
	{
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_TIM_IC_Init(&htim1) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 0;
	if (HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}
	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	if (HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
	{
		Error_Handler();
	}

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *tim_baseHandle)
{

	GPIO_InitTypeDef GPIO_InitStruct =
	{ 0 };
	if (tim_baseHandle->Instance == TIM1)
	{
		/* USER CODE BEGIN TIM1_MspInit 0 */

		/* USER CODE END TIM1_MspInit 0 */
		/* TIM1 clock enable */
		__HAL_RCC_TIM1_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**TIM1 GPIO Configuration
		 PA8     ------> TIM1_CH1
		 PA9     ------> TIM1_CH2
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* TIM1 interrupt Init */
		HAL_NVIC_SetPriority(TIM1_UP_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
		HAL_NVIC_SetPriority(TIM1_CC_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
		/* USER CODE BEGIN TIM1_MspInit 1 */

		/* USER CODE END TIM1_MspInit 1 */
	}
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *tim_baseHandle)
{

	if (tim_baseHandle->Instance == TIM1)
	{
		/* USER CODE BEGIN TIM1_MspDeInit 0 */

		/* USER CODE END TIM1_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM1_CLK_DISABLE();

		/**TIM1 GPIO Configuration
		 PA8     ------> TIM1_CH1
		 PA9     ------> TIM1_CH2
		 */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8 | GPIO_PIN_9);

		/* TIM1 interrupt Deinit */
		HAL_NVIC_DisableIRQ(TIM1_UP_IRQn);
		HAL_NVIC_DisableIRQ(TIM1_CC_IRQn);
		/* USER CODE BEGIN TIM1_MspDeInit 1 */

		/* USER CODE END TIM1_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */
#define FREQUENCY_CNT_CLK 8000000
#define ARR 65535

typedef enum
{
	WAIT_INITIAL_RISING_EDGE, WAIT_FALLING_EDGE, WAIT_PERIOD_END
} PeriodStateMachine;

PeriodStateMachine state = WAIT_INITIAL_RISING_EDGE;

uint32_t volatile overflowCounter = 0;

uint16_t volatile timestampStart = 0;
uint16_t volatile timestampFall = 0;
uint16_t volatile timestampEnd = 0;

uint32_t volatile ticksElapsedActive = 0;
uint32_t volatile ticksElapsedPeriod = 0;

uint32_t volatile frequency = 0;
uint32_t volatile dutyCycle = 0;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	switch (state)
	{
	case WAIT_INITIAL_RISING_EDGE:
		if (htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
//			timestampStart = TIM1->CCR1;
			timestampStart = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			overflowCounter = 0;
			state = WAIT_FALLING_EDGE;
		}
		break;
	case WAIT_FALLING_EDGE:
		if (htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			timestampFall = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
			ticksElapsedActive = (timestampFall + overflowCounter * (ARR + 1))
					- timestampStart;
			state = WAIT_PERIOD_END;
		}
		break;
	case WAIT_PERIOD_END:
		if (htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			timestampEnd = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			ticksElapsedPeriod = (timestampEnd + overflowCounter * (ARR + 1))
					- timestampStart;

			/*
			 * T = 1 / F
			 * F = 1 / T
			 *
			 * T = ticksElapsedPeriod * (1 / FREQUENCY_CNT_CLK)
			 * F = FREQUENCY_CNT_CLK / ticksElapsedPeriod
			 */

			frequency = FREQUENCY_CNT_CLK / ticksElapsedPeriod;
			dutyCycle = (ticksElapsedActive * 100) / ticksElapsedPeriod;

			timestampStart = timestampEnd;
			overflowCounter = 0;
			state = WAIT_PERIOD_END;
		}
		break;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	++overflowCounter;
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
