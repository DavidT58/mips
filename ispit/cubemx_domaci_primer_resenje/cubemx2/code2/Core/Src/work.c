/*
 * work.c
 *
 *  Created on: Jan 18, 2021
 *      Author: Marko Micovic
 */

#include "work.h"

#include "FreeRTOS.h"
#include "task.h"

#include "tim.h"

#include "uart_driver.h"
#include "lcd.h"

static void workTask(void *parameters)
{
	LCD_CommandEnqueue(LCD_INSTRUCTION,
	LCD_SET_CG_RAM_ADDRESS_INSTRUCTION | 0x08);

	LCD_CommandEnqueue(LCD_DATA, 0x1F);
	LCD_CommandEnqueue(LCD_DATA, 0x1F);
	LCD_CommandEnqueue(LCD_DATA, 0x1F);
	LCD_CommandEnqueue(LCD_DATA, 0x1F);
	LCD_CommandEnqueue(LCD_DATA, 0x1F);
	LCD_CommandEnqueue(LCD_DATA, 0x1F);
	LCD_CommandEnqueue(LCD_DATA, 0x1F);

	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

	htim3.Instance->CCR1 = 0;
	htim3.Instance->CCR2 = 0;

	while (1)
	{
		MotorCommand motorCommand = UART_BlockReceiveMotorCommand();
		switch (motorCommand.motor)
		{
		case 1:
			htim3.Instance->CCR1 = motorCommand.velocity;
			LCD_CommandEnqueue(LCD_INSTRUCTION,
			LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x00);
			break;
		case 2:
			htim3.Instance->CCR2 = motorCommand.velocity;
			LCD_CommandEnqueue(LCD_INSTRUCTION,
			LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x40);
			break;
		}

		for (uint32_t i = 0; i < 16; i++)
		{
			if (i < motorCommand.velocity)
			{
				LCD_CommandEnqueue(LCD_DATA, 0x01);
			}
			else
			{
				LCD_CommandEnqueue(LCD_DATA, ' ');
			}
		}
	}
}

void workInit()
{
	UART_Init();
	LCD_Init();
	xTaskCreate(workTask, "workTask", 128, NULL, 5, NULL);
}

