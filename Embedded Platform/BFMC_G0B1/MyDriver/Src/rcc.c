/*
 * rcc.c — G0B1 port
 *
 * G0B1 boots on HSI16 (16 MHz) by default with no PLL — SYSCLK = AHB = APB = 16 MHz.
 * No clock tree setup needed to match F411's 16 MHz peripheral assumption.
 */

#include "mydriver.h"

void RCC_Init(void)
{
    /* HSI16 is on by default at reset; just confirm it's ready */
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));
}
