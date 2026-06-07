/*
 * systick.c
 *
 *  Created on: Feb 14, 2026
 *      Author: Admin
 */

#include "mydriver.h"

volatile uint32_t ms_tick = 0;

void SysTick_Init(uint32_t cpu_freq)
{
    uint32_t reload = (cpu_freq / 1000) - 1;

    SYSTICK->LOAD = reload;   // Set reload value
    SYSTICK->VAL  = 0;        // Clear current value

    SYSTICK->CTRL = (1 << 2) | (1 << 1) | (1 << 0);  // CLKSOURCE + TICKINT + ENABLE
}

void Delay_t(uint32_t ms)
{
    while (ms--)
    {
        while ((SYSTICK->CTRL & (1U << 16)) == 0);
    }
}

void SysTick_Handler(void)
{
    ms_tick++;
}
