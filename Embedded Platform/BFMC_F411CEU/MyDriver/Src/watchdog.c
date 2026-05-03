/*
 * watchdog.c
 *
 *  Created on: Feb 5, 2026
 *      Author: Admin
 */

#include "mydriver.h"

void Watchdog_Init()
{
    IWDG->KR = IWDG_KR_UNLOCK;
    while (IWDG->SR & (1 << 0));   // Wait prescaler ready
    IWDG->PR = 0x06;                // Prescaler /256
    while (IWDG->SR & (1 << 1));   // Wait reload ready
    IWDG->RLR = 600;                // Set reload ~16s
    IWDG->KR = IWDG_KR_RELOAD;
    IWDG->KR = IWDG_KR_START;
}

void Feed_WD()
{
    IWDG->KR = IWDG_KR_RELOAD;
}
