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

static UART_HandleTypeDef *phuart = &huart1;

// TRANSMIT
// -----------------------------------------------------------------------------

static TaskHandle_t UART_TransmitTaskHandle;
static QueueHandle_t UART_TransmitQueueHandle;
static SemaphoreHandle_t UART_TransmitMutexHandle;

static void UART_TransmitTask(void *parameters)
{
	uint8_t buffer;
	while (1)
	{
		xQueueReceive(UART_TransmitQueueHandle, &buffer, portMAX_DELAY);
		HAL_UART_Transmit_IT(phuart, &buffer, sizeof(uint8_t));
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == phuart->Instance)
	{
		BaseType_t woken = pdFALSE;
		vTaskNotifyGiveFromISR(UART_TransmitTaskHandle, &woken);
		portYIELD_FROM_ISR(woken);
	}
}

// RECEIVE
// -----------------------------------------------------------------------------

static TaskHandle_t UART_ReceiveTaskHandle;
static QueueHandle_t UART_ReceiveQueueHandle;
static SemaphoreHandle_t UART_ReceiveMutexHandle;

static void UART_ReceiveTask(void *parameters)
{
	uint8_t buffer;
	while (1)
	{
		HAL_UART_Receive_IT(phuart, &buffer, sizeof(uint8_t));
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		xQueueSendToBack(UART_ReceiveQueueHandle, &buffer, portMAX_DELAY);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == phuart->Instance)
	{
		BaseType_t woken = pdFALSE;
		vTaskNotifyGiveFromISR(UART_ReceiveTaskHandle, &woken);
		portYIELD_FROM_ISR(woken);
	}
}

// GENERAL
// -----------------------------------------------------------------------------

void UART_Init()
{
	xTaskCreate(UART_TransmitTask, "transmitTask", 128, NULL, 4,
			&UART_TransmitTaskHandle);
	UART_TransmitQueueHandle = xQueueCreate(128, sizeof(uint8_t));
	UART_TransmitMutexHandle = xSemaphoreCreateMutex();

	xTaskCreate(UART_ReceiveTask, "receiveTask", 128, NULL, 20,
			&UART_ReceiveTaskHandle);
	UART_ReceiveQueueHandle = xQueueCreate(128, sizeof(uint8_t));
	UART_ReceiveMutexHandle = xSemaphoreCreateMutex();
}

// TRANSMIT UTIL
// -----------------------------------------------------------------------------

void UART_AsyncTransmitMotorCommand(MotorCommand motorCommand)
{
	xSemaphoreTake(UART_TransmitMutexHandle, portMAX_DELAY);

	xQueueSendToBack(UART_TransmitQueueHandle, &motorCommand.motor,
			portMAX_DELAY);
	xQueueSendToBack(UART_TransmitQueueHandle, &motorCommand.velocity,
			portMAX_DELAY);

	xSemaphoreGive(UART_TransmitMutexHandle);
}

// RECEIVE UTIL
// -----------------------------------------------------------------------------

MotorCommand UART_BlockReceiveMotorCommand()
{
	xSemaphoreTake(UART_ReceiveMutexHandle, portMAX_DELAY);

	MotorCommand motorCommand;

	xQueueReceive(UART_ReceiveQueueHandle, &motorCommand.motor,
	portMAX_DELAY);
	xQueueReceive(UART_ReceiveQueueHandle, &motorCommand.velocity,
	portMAX_DELAY);

	xSemaphoreGive(UART_ReceiveMutexHandle);

	return motorCommand;
}
