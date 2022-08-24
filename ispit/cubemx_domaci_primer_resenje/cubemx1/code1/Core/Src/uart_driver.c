/*
 * uart_driver.c
 *
 *  Created on: Jan 18, 2021
 *      Author: Marko Micovic
 */

#include "uart_driver.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <string.h>

#include "usart.h"

static UART_HandleTypeDef *phuart[2] =
{ &huart1, &huart2 };

// TRANSMIT
// -----------------------------------------------------------------------------

static TaskHandle_t UART_TxTaskHandle[2];
static QueueHandle_t UART_TxQueueHandle[2];
static SemaphoreHandle_t UART_TxMutexHandle[2];

static void UART_TxTask(void *parameters)
{
	UART_Target target = (UART_Target) parameters;

	uint8_t buffer;
	while (1)
	{
		xQueueReceive(UART_TxQueueHandle[target], &buffer, portMAX_DELAY);
		HAL_UART_Transmit_IT(phuart[target], &buffer, sizeof(uint8_t));
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	BaseType_t woken = pdFALSE;

	if (huart->Instance == phuart[MCU]->Instance)
	{
		vTaskNotifyGiveFromISR(UART_TxTaskHandle[MCU], &woken);
	}

	if (huart->Instance == phuart[VTE]->Instance)
	{
		vTaskNotifyGiveFromISR(UART_TxTaskHandle[VTE], &woken);
	}

	portYIELD_FROM_ISR(woken);
}

// RECEIVE
// -----------------------------------------------------------------------------

static TaskHandle_t UART_RxTaskHandle[2];
static QueueHandle_t UART_RxQueueHandle[2];
static SemaphoreHandle_t UART_RxMutexHandle[2];

static void UART_RxTask(void *parameters)
{
	UART_Target target = (UART_Target) parameters;

	uint8_t buffer;
	while (1)
	{
		HAL_UART_Receive_IT(phuart[target], &buffer, sizeof(uint8_t));
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		xQueueSendToBack(UART_RxQueueHandle[target], &buffer, portMAX_DELAY);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	BaseType_t woken = pdFALSE;

	if (huart->Instance == phuart[MCU]->Instance)
	{
		vTaskNotifyGiveFromISR(UART_RxTaskHandle[MCU], &woken);
	}

	if (huart->Instance == phuart[VTE]->Instance)
	{
		vTaskNotifyGiveFromISR(UART_RxTaskHandle[VTE], &woken);
	}

	portYIELD_FROM_ISR(woken);
}

// GENERAL
// -----------------------------------------------------------------------------

void UART_Init()
{
	// Transmit
	xTaskCreate(UART_TxTask, "transmitTask_MCU", 64, (void*) MCU, 4,
			&UART_TxTaskHandle[MCU]);
	UART_TxQueueHandle[MCU] = xQueueCreate(64, sizeof(uint8_t));
	UART_TxMutexHandle[MCU] = xSemaphoreCreateMutex();

	xTaskCreate(UART_TxTask, "transmitTask_VTE", 64, (void*) VTE, 4,
			&UART_TxTaskHandle[VTE]);
	UART_TxQueueHandle[VTE] = xQueueCreate(64, sizeof(uint8_t));
	UART_TxMutexHandle[VTE] = xSemaphoreCreateMutex();

	// Receive
	xTaskCreate(UART_RxTask, "receiveTask_MCU", 64, (void*) MCU, 20,
			&UART_RxTaskHandle[MCU]);
	UART_RxQueueHandle[MCU] = xQueueCreate(64, sizeof(uint8_t));
	UART_RxMutexHandle[MCU] = xSemaphoreCreateMutex();

	xTaskCreate(UART_RxTask, "receiveTask_VTE", 64, (void*) VTE, 20,
			&UART_RxTaskHandle[VTE]);
	UART_RxQueueHandle[VTE] = xQueueCreate(64, sizeof(uint8_t));
	UART_RxMutexHandle[VTE] = xSemaphoreCreateMutex();
}

// TRANSMIT UTIL
// -----------------------------------------------------------------------------

void UART_AsyncTxString(UART_Target target, char const *string)
{
	if (string != NULL)
	{
		xSemaphoreTake(UART_TxMutexHandle[target], portMAX_DELAY);

		for (uint32_t i = 0; i < strlen(string); i++)
		{
			xQueueSendToBack(UART_TxQueueHandle[target], string + i,
					portMAX_DELAY);
		}

		xSemaphoreGive(UART_TxMutexHandle[target]);
	}
}

void UART_AsyncTxMotorCommand(UART_Target target, MotorCommand motorCommand)
{
	xSemaphoreTake(UART_TxMutexHandle[target], portMAX_DELAY);

	xQueueSendToBack(UART_TxQueueHandle[target], &motorCommand.motor,
			portMAX_DELAY);
	xQueueSendToBack(UART_TxQueueHandle[target], &motorCommand.velocity,
			portMAX_DELAY);

	xSemaphoreGive(UART_TxMutexHandle[target]);
}

// RECEIVE UTIL
// -----------------------------------------------------------------------------

char* UART_BlockRxString(UART_Target target)
{
	xSemaphoreTake(UART_RxMutexHandle[target], portMAX_DELAY);

	char *string = pvPortMalloc(64);

	if (string != NULL)
	{
		uint32_t index = 0;

		char character = '\0';
		while (character != '\r' && index < 64)
		{
			xQueueReceive(UART_RxQueueHandle[target], &character,
			portMAX_DELAY);
			string[index++] = character;
		}

		string[--index] = '\0';
	}

	xSemaphoreGive(UART_RxMutexHandle[target]);

	return string;
}

MotorCommand UART_BlockRxMotorCommand(UART_Target target)
{
	xSemaphoreTake(UART_RxMutexHandle[target], portMAX_DELAY);

	MotorCommand motorCommand;

	xQueueReceive(UART_RxQueueHandle[target], &motorCommand.motor,
	portMAX_DELAY);
	xQueueReceive(UART_RxQueueHandle[target], &motorCommand.velocity,
	portMAX_DELAY);

	xSemaphoreGive(UART_RxMutexHandle[target]);

	return motorCommand;
}
