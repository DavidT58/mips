#include "driver_lcd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"
#include "queue.h"

static TaskHandle_t lcdTask;
static QueueHandle_t lcdQueue;
static void LCD_Write(command com,val val){
	GPIOC->ODR = ((com & 0x01) << 5 | (val & 0x0F));
	GPIOC->ODR |= 0x40;
	GPIOC->ODR &= ~(0x40);
}

static void LCD_CommandInitiate(command com,val val){
	LCD_Write(com,val >> 4);
	LCD_Write(com,val);
	vTaskDelay(pdMS_TO_TICKS(2));
}

static void lcd_task(void *params){
	vTaskDelay(pdMS_TO_TICKS(20));

	LCD_Write(LCD_INSTRUCTION,
			(LCD_FUNCTION_SET_INSTRUCTION | LCD_FUNCTION_SET_4_BIT_INTERFACE)
					>> 4);
	vTaskDelay(pdMS_TO_TICKS(2));

	LCD_CommandInitiate(LCD_INSTRUCTION,
	LCD_FUNCTION_SET_INSTRUCTION |
	LCD_FUNCTION_SET_4_BIT_INTERFACE |
	LCD_FUNCTION_SET_2_LINE |
	LCD_FUNCTION_SET_5x8_DOTS);

	LCD_CommandInitiate(LCD_INSTRUCTION,
	LCD_CONTROL_INSTRUCTION |
	LCD_CONTROL_DISPLAY_ON |
	LCD_CONTROL_CURSOR_OFF |
	LCD_CONTROL_BLINK_OFF);

	LCD_CommandInitiate(LCD_INSTRUCTION,
	LCD_ENTRY_MODE_INSTRUCTION |
	LCD_ENTRY_MODE_INC_ADR |
	LCD_ENTRY_MODE_SHIFT_OFF);

	LCD_CommandInitiate(LCD_INSTRUCTION, LCD_RETURN_HOME_INSTRUCTION);

	commandStruct commandStruct;
	while(1){
		xQueueReceive(lcdQueue, &commandStruct,portMAX_DELAY);
		LCD_CommandInitiate(commandStruct.command, commandStruct.val);
	}
}



void lcd_init(){
	lcdQueue = xQueueCreate(64,sizeof(commandStruct));
	xTaskCreate(lcd_task, "lcd_task", 64, NULL, 2, &lcdTask);
}

void lcd_put(command com,val val){
	commandStruct commandStruct={
			com,val
	};
	xQueueSend(lcdQueue, &commandStruct,portMAX_DELAY);
}

void lcd_putISR(command com,val val, BaseType_t* higherPri){
	commandStruct commandStruct={
			com,val
	};

	xQueueSendFromISR(lcdQueue, &commandStruct,higherPri);
}

