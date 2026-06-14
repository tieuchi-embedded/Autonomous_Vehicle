/*
 * timer.c — G0B1 port
 *
 * APB clock = 16 MHz (HSI16, no PLL) — PSC recalculated for 16MHz tick base
 * (F411 used PSC=84-1 because TIM3 ran on an 84MHz APB1 timer clock there).
 */

#include "mydriver.h"

void Timer2_Init()
{
    RCC->APBENR1 |= RCC_APBENR1_TIM2EN;
    TIM2->PSC = SYSCLK_MHZ;
    TIM2->ARR = 1;
    TIM2->CR1 |= TIM_CR1_CEN;
}

void Timer3_Init(void)
{
    RCC->APBENR1 |= RCC_APBENR1_TIM3EN;
    TIM3->CR1 = 0;
    TIM3->PSC = SYSCLK_MHZ - 1;     // 16MHz / 16 = 1 MHz
    TIM3->ARR = 1000 - 1;   // 1 ms
    TIM3->EGR = 1;          // LOAD PSC, ARR
    TIM3->SR &= ~1;         // CLEAR UIF
    TIM3->CR1 |= 1;         // CEN
}
