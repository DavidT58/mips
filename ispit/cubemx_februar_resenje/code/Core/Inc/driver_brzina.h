/*
 * driver_brzina.h
 *
 *  Created on: Feb 16, 2022
 *      Author: Nikola
 */

#ifndef CORE_INC_DRIVER_BRZINA_H_
#define CORE_INC_DRIVER_BRZINA_H_


extern void brzina_init();
extern float getBrzina();
extern void transmit_char(char);
extern void transmit_digit(uint32_t);
#endif /* CORE_INC_DRIVER_BRZINA_H_ */
