/*
 * uart_driver.h
 *
 *  Created on: Jan 18, 2021
 *      Author: Marko Micovic
 */

#ifndef CORE_INC_UART_DRIVER_H_
#define CORE_INC_UART_DRIVER_H_

#include <stdint.h>

#include "work.h"

typedef enum
{
	VTE = 0, MCU
} UART_Target;

extern void UART_Init();

extern void UART_AsyncTxString(UART_Target target, char const *string);
extern void UART_AsyncTxMotorCommand(UART_Target target, MotorCommand command);

extern char* UART_BlockRxString(UART_Target target);
extern MotorCommand UART_BlockRxMotorCommand(UART_Target target);

#endif /* CORE_INC_UART_DRIVER_H_ */
