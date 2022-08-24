/*
 * example.c
 *
 *  Created on: Dec 28, 2020
 *      Author: Marko Micovic
 */

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#include "example.h"
#include "gpio.h"

QueueHandle_t queueHandle;

TimerHandle_t timer1;
TimerHandle_t timer2;

void callbackTimer(TimerHandle_t timerHandle);
void taskToggler(void *parameters);

void exampleInit()
{
	queueHandle = xQueueCreate(10, sizeof(uint32_t));
	xTaskCreate(taskToggler, "taskToggler", 128, NULL, 2, NULL);

	timer1 = xTimerCreate("seconds1", pdMS_TO_TICKS(1000), pdTRUE,
			(void*) GPIO_PIN_0, callbackTimer);
	timer2 = xTimerCreate("seconds3", pdMS_TO_TICKS(3000), pdTRUE,
			(void*) GPIO_PIN_1, callbackTimer);

	xTimerStart(timer1, portMAX_DELAY);
	xTimerStart(timer2, portMAX_DELAY);
}

void callbackTimer(TimerHandle_t timerHandle)
{
	uint32_t GPIO_Pin = (uint32_t) pvTimerGetTimerID(timerHandle);
	HAL_GPIO_TogglePin(GPIOC, GPIO_Pin);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_2 || GPIO_Pin == GPIO_PIN_3)
	{
		uint32_t itemToQueue = GPIO_Pin;
		BaseType_t woken = pdFALSE;
		xQueueSendToBackFromISR(queueHandle, &itemToQueue, &woken);
		portYIELD_FROM_ISR(woken);
	}
}

void taskToggler(void *parameters)
{
	uint32_t bufferForPinIndex;
	while (1)
	{
		xQueueReceive(queueHandle, &bufferForPinIndex, portMAX_DELAY);
		HAL_GPIO_TogglePin(GPIOC, bufferForPinIndex);
	}
}

