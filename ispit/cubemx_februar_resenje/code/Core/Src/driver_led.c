#include "driver_led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver_brzina.h"
#include "gpio.h"

static void led_task(void* params){
	while(1){

		uint32_t brzina = getBrzina();
		if(brzina > 50){
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_13);
		}else{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_RESET);
		}
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void led_init(){
	xTaskCreate(led_task, "led_task", 64, NULL, 10, NULL);
}

