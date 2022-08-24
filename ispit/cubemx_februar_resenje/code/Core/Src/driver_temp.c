#include "driver_temp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "adc.h"
#include "queue.h"

static TaskHandle_t tempTaskHandle;
static QueueHandle_t mailBox;

float getTemp(){
	float temp;
	xQueuePeek(mailBox, &temp, portMAX_DELAY);
	return temp;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	BaseType_t* woken = pdFALSE;
	vTaskNotifyGiveFromISR(tempTaskHandle, woken);
	portYIELD_FROM_ISR(woken);
}

static void tempTask(void *params){
	while(1){
		HAL_ADC_Start_IT(&hadc1);
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		float tekTemp = HAL_ADC_GetValue(&hadc1);
		tekTemp *= 5.0 / 4096.0;
		tekTemp *= 100;
		xQueueOverwrite(mailBox, &tekTemp);
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}


void initTemp(){
	mailBox = xQueueCreate(1,sizeof(float));
	xTaskCreate(tempTask, "tempTask", 64, NULL, 2, &tempTaskHandle);
}

