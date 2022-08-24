#include "driver_brzina.h"
#include "FreeRTOS.h"
#include "tim.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "usart.h"

typedef enum{
	begin = 0,
	end,
	falling
}stanja;

stanja stanje = begin;
uint32_t start, endTime, elapsed, fall, elapsedActive, freq = 0, duty;
extern uint32_t overflow;
 void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	 switch(stanje){

	 case begin:
		 if(htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){
			 start = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			 overflow = 0;
			 stanje = falling;
		 }
		 break;

	 case falling:
	 		 if(htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2){
	 			 fall = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
	 			 if(overflow > 0)
	 				 elapsedActive = fall + 65536 - start;
	 			 else
	 				elapsedActive = fall - start;
	 			 stanje = end;
	 		 }
	 		 break;

	 case end:
		 if(htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){
			 endTime = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			 if(overflow > 0)
				 elapsed = endTime + 65536 - start;
			 else
				 elapsed = endTime - start;

			 freq = 8000/elapsed;
			 overflow = 0;
			 duty = elapsed * 100 / elapsedActive;
			 start = endTime;
		 }
		 break;
	 }
}

 void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
 {
 	 if (htim->Instance == TIM2) {
 	     HAL_IncTick();
 	   }else{
 		   overflow++;
 	   }
 }

 float getBrzina(){
	 return freq * 2.4;
 }

static TaskHandle_t uartHandle;
static QueueHandle_t uartQueueHandle;
static SemaphoreHandle_t uartSem;


static void uartTask(void* params){
	uint8_t buffer;
	while(1){
		xQueueReceive(uartQueueHandle, &buffer,portMAX_DELAY);
		HAL_UART_Transmit_IT(&huart1, &buffer, sizeof(uint8_t));
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

void brzina_init(){
	HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
	HAL_TIM_Base_Start_IT(&htim1);
	xTaskCreate(uartTask, "uartTask", 64, NULL, 2, &uartHandle);
	uartQueueHandle = xQueueCreate(64,sizeof(uint8_t));
	uartSem = xSemaphoreCreateMutex();
}




void transmit_char(char c){
	xSemaphoreTake(uartSem,portMAX_DELAY);
	xQueueSendToBack(uartQueueHandle, &c, portMAX_DELAY);
	xSemaphoreGive(uartSem);
}

void transmit_digit(uint32_t num){
	xSemaphoreTake(uartSem,portMAX_DELAY);
	char niz[32];
	uint32_t index = 32;
	while(num > 0 && index >= 0){

		niz[--index] = '0' + num % 10;
		num /= 10;
	}

	for(uint32_t i = index; i < 32; i++){
		xQueueSendToBack(uartQueueHandle,&niz[i],portMAX_DELAY);
	}
	xSemaphoreGive(uartSem);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == huart1.Instance){
		BaseType_t woken = pdFALSE;
		vTaskNotifyGiveFromISR(uartHandle, &woken);
		portYIELD_FROM_ISR(woken);
	}
}
