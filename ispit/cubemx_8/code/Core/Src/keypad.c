/*
 * keypad.c
 *
 *  Created on: Jan 4, 2021
 *      Author: Marko Micovic
 */

#include "keypad.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "lcd.h"

#include "gpio.h"

static char KEY_Map[4][3] =
{
{ '1', '2', '3' },
{ '4', '5', '6' },
{ '7', '8', '9' },
{ '*', '0', '#' } };

static TimerHandle_t KEY_TimerHandle;
static TaskHandle_t KEY_TaskHandle;

static uint8_t KEY_PreviousReleased = 1;

static void KEY_Task(void *parameters)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (KEY_PreviousReleased)
		{
			for (uint8_t row = 0; row < 4; row++)
			{
				GPIOB->ODR = 0x01 << row;

				// wait for signal to stabilize

				uint32_t state = (GPIOB->IDR >> 4) & 0x07;

				for (uint8_t column = 0; column < 3; column++)
				{
					if (state & (0x01 << column))
					{
						KEY_PreviousReleased = 0;
						xTimerStart(KEY_TimerHandle, portMAX_DELAY);
						LCD_CommandEnqueue(LCD_DATA, KEY_Map[row][column]);
					}
				}
			}
			GPIOB->ODR = 0x0F;
		}
	}
}

static void KEY_TimerCallback(TimerHandle_t handle)
{
	if (!KEY_PreviousReleased)
	{
		if (((GPIOB->IDR >> 4) & 0x07) == 0)
		{
			KEY_PreviousReleased = 1;
		}
		else
		{
			xTimerStart(KEY_TimerHandle, portMAX_DELAY);
		}
	}
}

void KEY_Init()
{
	GPIOB->ODR = 0x0F;
	KEY_TimerHandle = xTimerCreate("KEY_TimerHandle", pdMS_TO_TICKS(50),
	pdFALSE, NULL, KEY_TimerCallback);
	xTaskCreate(KEY_Task, "KEY_Task", 128, NULL, 10, &KEY_TaskHandle);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_7)
	{
		BaseType_t taskWoken = pdFALSE;
		vTaskNotifyGiveFromISR(KEY_TaskHandle, &taskWoken);
		portYIELD_FROM_ISR(taskWoken);
	}
}

