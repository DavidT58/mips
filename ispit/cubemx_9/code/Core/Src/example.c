/*
 * example.c
 *
 *  Created on: Jan 6, 2021
 *      Author: Marko Micovic
 */

#include "example.h"

#include "FreeRTOS.h"
#include "task.h"

#include "uart_driver.h"

#include "gpio.h"

static void exampleTask(void *parameters)
{
//	UART_AsyncTransmitCharacter('M');
//	UART_AsyncTransmitString("IPS_");
//	UART_AsyncTransmitDecimal(2020);
//	UART_AsyncTransmitCharacter('\r');
//
//	char character = UART_BlockReceiveCharacter();
//	UART_AsyncTransmitCharacter(character);
//
//	char *string = UART_BlockReceiveString();
//	if (string != NULL)
//	{
//		UART_AsyncTransmitString(string);
//		vPortFree(string);
//	}

	while (1)
	{
		UART_AsyncTransmitString("Uneti redni broj diode [1-4]\r");
		uint32_t index = UART_BlockReceiveDecimal();
		if (index >= 1 && index <= 4)
		{
			HAL_GPIO_TogglePin(GPIOC, 0x01 << (index - 1));
		}
		else
		{
			UART_AsyncTransmitString("Redni broj izvan opsega!\r\r");
		}
	}
}

void exampleInit()
{
	UART_Init();
	xTaskCreate(exampleTask, "exampleTask", 128, NULL, 10, NULL);
}
