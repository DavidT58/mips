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

extern void UART_Init();

extern void UART_AsyncTransmitMotorCommand(MotorCommand motorCommand);

extern MotorCommand UART_BlockReceiveMotorCommand();

#endif /* CORE_INC_UART_DRIVER_H_ */
