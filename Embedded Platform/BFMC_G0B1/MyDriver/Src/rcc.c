/*
 * rcc.c — G0B1 port
 *
 * G0B1 boots on HSI16 (16 MHz) by default with no PLL — SYSCLK = AHB = APB = 16 MHz.
 * No clock tree setup needed to match F411's 16 MHz peripheral assumption.
 */

#include "mydriver.h"


void RCC_Init(void)
{
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

#if USE_PLL
    RCC->PLLCFGR = (0b10      << 0)
                 | (PLL_M_REG << 4)
                 | (PLL_N     << 8)
                 | (PLL_R_REG << 29);
    RCC->PLLCFGR |= (1U << 28);        // PLLREN
    RCC->CR      |= (1U << 24);        // PLLON
    while (!(RCC->CR & (1U << 25)));   // Wait PLLRDY
#endif

    // Flash latency
    *((volatile uint32_t*)0x40022000U) =
        (*((volatile uint32_t*)0x40022000U) & ~0x7U) | FLASH_LATENCY;

#if USE_PLL
    RCC->CFGR = (RCC->CFGR & ~0x3U) | 0x2U;
    while (((RCC->CFGR >> 3) & 0x3U) != 0x2U);
#endif
}
