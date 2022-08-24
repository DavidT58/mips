/*
 * example.c
 *
 *  Created on: Jan 4, 2021
 *      Author: Marko Micovic
 */

#include "example.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lcd.h"
#include "keypad.h"

static void exampleTask(void *parameters)
{
	// LCD example
	// -------------------------------------------------------------------

	// Instructions (page: 23)

	// Write to DDRAM (Line1)
	char line1[16] = "Mikroprocesorski";
	LCD_CommandEnqueue(LCD_INSTRUCTION,
	LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x00);
	for (uint32_t i = 0; i < 16; i++)
	{
		LCD_CommandEnqueue(LCD_DATA, line1[i]);
		vTaskDelay(pdMS_TO_TICKS(100));
	}

	// Turn on Cursor
	LCD_CommandEnqueue(LCD_INSTRUCTION,
	LCD_CONTROL_INSTRUCTION |
	LCD_CONTROL_DISPLAY_ON |
	LCD_CONTROL_CURSOR_ON |
	LCD_CONTROL_BLINK_OFF);

	// Write to DDRAM (Line2)
	char line2[16] = "sistemi";
	LCD_CommandEnqueue(LCD_INSTRUCTION,
	LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x40);
	for (uint32_t i = 0; i < 16; i++)
	{
		LCD_CommandEnqueue(LCD_DATA, line2[i]);
		vTaskDelay(pdMS_TO_TICKS(100));
	}

	// Turn on Blink
	LCD_CommandEnqueue(LCD_INSTRUCTION,
	LCD_CONTROL_INSTRUCTION |
	LCD_CONTROL_DISPLAY_ON |
	LCD_CONTROL_CURSOR_ON |
	LCD_CONTROL_BLINK_ON);

	// Change DDRAM address
	LCD_CommandEnqueue(LCD_INSTRUCTION,
	LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x03);

//	// Shift display
//	for (uint32_t i = 0; i < 10; i++)
//	{
//		LCD_CommandEnqueue(LCD_INSTRUCTION,
//		LCD_SHIFT_INSTRUCTION |
//		LCD_SHIFT_DISPLAY |
//		LCD_SHIFT_DIRECTION_RIGHT);
//		vTaskDelay(pdMS_TO_TICKS(100));
//	}

	// Write to CGRAM
	LCD_CommandEnqueue(LCD_INSTRUCTION,
	LCD_SET_CG_RAM_ADDRESS_INSTRUCTION | 0x08);

	LCD_CommandEnqueue(LCD_DATA, 0x04); //		_ _ X _ _
	LCD_CommandEnqueue(LCD_DATA, 0x02); //		_ _ _ X _
	LCD_CommandEnqueue(LCD_DATA, 0x11); //		X _ _ _ X
	LCD_CommandEnqueue(LCD_DATA, 0x01); //		_ _ _ _ X
	LCD_CommandEnqueue(LCD_DATA, 0x11); //		X _ _ _ X
	LCD_CommandEnqueue(LCD_DATA, 0x02); //		_ _ _ X _
	LCD_CommandEnqueue(LCD_DATA, 0x04); //		_ _ X _ _

	LCD_CommandEnqueue(LCD_INSTRUCTION,
	LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x48);

	LCD_CommandEnqueue(LCD_DATA, 0x01);

	// Keypad example
	// -------------------------------------------------------------------

	LCD_CommandEnqueue(LCD_INSTRUCTION,
	LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x00);

	KEY_Init();

	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void exampleInit()
{
	LCD_Init();
	xTaskCreate(exampleTask, "exampleTask", 128, NULL, 5, NULL);
}

