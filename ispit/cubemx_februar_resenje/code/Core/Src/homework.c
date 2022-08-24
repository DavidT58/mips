#include "homework.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver_lcd.h"
#include "driver_temp.h"
#include <stdlib.h>
#include <string.h>
#include "gpio.h"
#include "driver_brzina.h"
#include "driver_led.h"

static uint32_t broj;
static char text1[4], text2[4];

static void homeworkTask(void* params){
	char temper[8] = "Temper: ";
	char brzina[8] = "Brzina: ";
	lcd_put(LCD_INSTRUCTION,
			LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x90);
	for(uint32_t i = 0;i < 8;i++){
		lcd_put(LCD_DATA, temper[i]);
	}


	lcd_put(LCD_INSTRUCTION,
				LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x40);
	for(uint32_t i = 0;i < 8;i++){
		lcd_put(LCD_DATA, brzina[i]);

	}

	while(1){
		broj = getTemp();
		transmit_digit(broj);
		itoa(broj, text1, 10);
		lcd_put(LCD_INSTRUCTION,
				LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x99);
		for(uint32_t i = 0;i < strlen(text1);i++){
			lcd_put(LCD_DATA, text1[i]);
		}

		transmit_char(' ');
		transmit_char('/');
		transmit_char(' ');

		broj = getBrzina();
		transmit_digit(broj);
		itoa(broj, text2, 10);
		lcd_put(LCD_INSTRUCTION,
				LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x49);

		for(uint32_t i = 0;i < strlen(text2);i++){
			lcd_put(LCD_DATA, text2[i]);
		}

		vTaskDelay(pdMS_TO_TICKS(200));

		lcd_put(LCD_INSTRUCTION,
				LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x99);
		for(uint32_t i = 0;i < strlen(text1);i++){
			lcd_put(LCD_DATA,'\b');
			transmit_char('\b');
		}

		transmit_char('\b'); //brise space
		transmit_char('\b'); //brise /
		transmit_char('\b'); //brise space

		lcd_put(LCD_INSTRUCTION,
				LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x49);
		for(uint32_t i = 0;i < strlen(text2);i++){
			lcd_put(LCD_DATA, '\b');
			transmit_char('\b');
		}

	}
}

void init(){
	lcd_init();
	initTemp();
	brzina_init();
	led_init();
	xTaskCreate(homeworkTask, "homeworkTask", 64, NULL, 5, NULL);
}


