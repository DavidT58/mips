/*
 * work.c
 *
 *  Created on: Jan 18, 2021
 *      Author: Marko Micovic
 */

#include "work.h"

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>

#include "uart_driver.h"

uint8_t velocity[2] =
{ 0, 0 };

static void workTask(void *parameters)
{
	while (1)
	{
		UART_AsyncTxString(VTE, "Unesite komandu:");
		char *input = UART_BlockRxString(VTE);

		if (input != NULL)
		{
			if (strlen(input) == 2 && '1' <= input[0] && input[0] <= '2'
					&& (input[1] == 'd' || input[1] == 'i'))
			{
				uint8_t motor = input[0] - '0';
				switch (input[1])
				{
				case 'd':
					// decrease
					if (velocity[motor - 1] > 0)
					{
						velocity[motor - 1]--;
					}
					break;
				case 'i':
					// increase
					if (velocity[motor - 1] < 16)
					{
						velocity[motor - 1]++;
					}
					break;
				}
				MotorCommand motorCommand =
				{ .motor = motor, .velocity = velocity[motor - 1] };
				UART_AsyncTxMotorCommand(MCU, motorCommand);
			}
			else
			{
				UART_AsyncTxString(VTE, "Pogresan format komande!\r");
			}

			vPortFree(input);
		}
	}
}

void workInit()
{
	UART_Init();
	xTaskCreate(workTask, "workTask", 128, NULL, 5, NULL);
}

