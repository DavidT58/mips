/*
 * casovnik.c
 *
 *  Created on: Nov 26, 2020
 *      Author: Marko Micovic
 */

#include "casovnik.h"

/*
 * Niz vrednosti koje je potrebno postaviti
 * na katode sedmosegmentnog displeja sa zajednickom anodom
 * kako bi se prikazala odgovarajuca decimalna cifra
 * ukoliko je povezivanje pinova izvrseno na naznacen nacin.
 *
 *                              A
 *                           *******
 *                         *         *
 *                       F *         * B
 *                         *    G    *
 *                           *******
 *                         *         *
 *                       E *         * C
 *                         *    D    *
 *                           *******
 *
 *
 * |-----|||-----|-----|-----|-----|-----|-----|-----|-----|||-----|
 * |     ||| PC7 | PC6 | PC5 | PC4 | PC3 | PC2 | PC1 | PC0 |||     |
 * |-----|||-----|-----|-----|-----|-----|-----|-----|-----|||-----|
 * | DIG ||| DP  |  A  |  B  |  C  |  D  |  E  |  F  |  G  ||| VAL |
 * |-----|||-----|-----|-----|-----|-----|-----|-----|-----|||-----|
 * |  0  |||  1  |  0  |  0  |  0  |  0  |  0  |  0  |  1  ||| 81  |
 * |  1  |||  1  |  1  |  0  |  0  |  1  |  1  |  1  |  1  ||| CF  |
 * |  2  |||  1  |  0  |  0  |  1  |  0  |  0  |  1  |  0  ||| 92  |
 * |  3  |||  1  |  0  |  0  |  0  |  0  |  1  |  1  |  0  ||| 86  |
 * |  4  |||  1  |  1  |  0  |  0  |  1  |  1  |  0  |  0  ||| CC  |
 * |  5  |||  1  |  0  |  1  |  0  |  0  |  1  |  0  |  0  ||| A4  |
 * |  6  |||  1  |  0  |  1  |  0  |  0  |  0  |  0  |  0  ||| A0  |
 * |  7  |||  1  |  0  |  0  |  0  |  1  |  1  |  1  |  1  ||| 8F  |
 * |  8  |||  1  |  0  |  0  |  0  |  0  |  0  |  0  |  0  ||| 80  |
 * |  9  |||  1  |  0  |  0  |  0  |  0  |  1  |  0  |  0  ||| 84  |
 * |-----|||-----|-----|-----|-----|-----|-----|-----|-----|||-----|
 *
 */
uint8_t seven_seg[] =
{ 0x81, 0xCF, 0x92, 0x86, 0xCC, 0xA4, 0xA0, 0x8F, 0x80, 0x84 };

TIM_HandleTypeDef *casovnik_tim_handle = &htim1;

//extern void casovnik_TIM_odbrojao(TIM_HandleTypeDef *htim);

void casovnik()
{
//	HAL_TIM_Base_Start(casovnik_tim_handle);
//	for (int i = 0; i < 5; i++)
//	{
//		while ((casovnik_tim_handle->Instance->SR & TIM_SR_UIF) == 0)
//		{
//			// busy wait
//		}
//		casovnik_tim_handle->Instance->SR = ~TIM_SR_UIF;
//		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
//	}
//	HAL_TIM_Base_Stop(casovnik_tim_handle);

//	HAL_TIM_RegisterCallback(casovnik_tim_handle, HAL_TIM_PERIOD_ELAPSED_CB_ID,
//			&casovnik_TIM_odbrojao);

	HAL_TIM_Base_Start_IT(casovnik_tim_handle);

	GPIOC->ODR = seven_seg[1];
}

uint32_t counter = 0;
uint32_t sekunde = 0;
uint32_t minuti = 0;

uint8_t tekuca = 0;
uint8_t cifre[4] =
{ 0, 0, 0, 0 };

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == casovnik_tim_handle->Instance)
	{
		if (++counter == 100)
		{
			counter = 0;
			if (++sekunde == 60)
			{
				sekunde = 0;
				if (++minuti == 60)
				{
					minuti = 0;
				}
			}

			cifre[0] = minuti / 10;
			cifre[1] = minuti % 10;
			cifre[2] = sekunde / 10;
			cifre[3] = sekunde % 10;
		}

		tekuca = (tekuca + 1) % 4;

		GPIOC->ODR &= ~0xF00;

		GPIOC->ODR &= ~0xFF;
		GPIOC->ODR |= seven_seg[cifre[tekuca]];

		GPIOC->ODR |= 0x1 << (8 + tekuca);

	}
}

//void casovnik_TIM_odbrojao(TIM_HandleTypeDef *htim)
//{
//	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
//	if (++counter == 5)
//	{
//		HAL_TIM_Base_Stop_IT(casovnik_tim_handle);
//	}
//}
