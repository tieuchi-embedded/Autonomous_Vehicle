/*
 * timer.c
 *
 *  Created on: Jan 28, 2026
 *      Author: Admin
 */

#include "mydriver.h"

void Timer2_Init()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 16;
    TIM2->ARR = 1;
    TIM2->CR1 |= TIM_CR1_CEN;
}

void Timer3_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3->CR1 = 0;
    TIM3->PSC = 84 - 1;    // 1 MHz
    TIM3->ARR = 1000 - 1;  // 1 ms
    TIM3->EGR = 1;          // LOAD PSC, ARR
    TIM3->SR &= ~1;         // CLEAR UIF
    TIM3->CR1 |= 1;         // CEN
}
