/*
 * servo.c
 *
 *  Created on: Jan 28, 2026
 *      Author: Admin
 */

#include "mydriver.h"

void PWM_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    GPIOA->MODER &= ~(0b11 << (0 * 2));  // clear mode PA0
    GPIOA->MODER |=  (0b10 << (0 * 2));  // set AF mode
    GPIOA->AFR[0] &= ~(0xF << (0 * 4));
    GPIOA->AFR[0] |=  (0x1 << (0 * 4)); // AF1 (TIM2_CH1)

    GPIOA->MODER &= ~(0b11 << (1 * 2));
    GPIOA->MODER |=  (0b10 << (1 * 2));
    GPIOA->AFR[0] &= ~(0xF << (1 * 4));
    GPIOA->AFR[0] |=  (0x1 << (1 * 4)); // AF1

    TIM2->PSC = 16 - 1;     // chia 16MHz/16 = 1MHz tick
    TIM2->ARR = 20000 - 1;  // chu kỳ 20ms => PWM 50Hz
    TIM2->CCR1 = 1500;      // Servo speed stop (1.5 ms)
    TIM2->CCR2 = 1500;      // Servo steering center

    TIM2->CCMR1 &= ~((0xFF << 0) | (0xFF << 8));
    // CH1
    TIM2->CCMR1 |= (6 << 4) | (1 << 3);   // OC1M, OC1PE
    // CH2
    TIM2->CCMR1 |= (6 << 12) | (1 << 11); // OC2M, OC2PE

    TIM2->CCER |= (1 << 0);   // CC1E
    TIM2->CCER |= (1 << 4);   // CC2E

    TIM2->EGR = 1;             // UG=1

    TIM2->CR1 |= TIM_CR1_CEN;
}

void Servo_Speed_Set(uint16_t us)
{
    TIM2->CCR1 = us;
}

void Servo_Steering_Set(uint16_t us)
{
    TIM2->CCR2 = us;
}

// rpm: -500..+500, maps to 1000..2000 us (0 -> 1500)
void Servo_Speed_Set_RPM(float rpm)
{
    int pwm = 1500 - (int)rpm;
    if (pwm < 1000) pwm = 1000;
    if (pwm > 2000) pwm = 2000;
    Servo_Speed_Set((uint16_t)pwm);
}

// deg: -40..+40, maps to 1100..1900 us (0 -> 1500, 1 deg = 10 us)
void Servo_Steering_Set_Deg(float deg)
{
    int pwm = 1500 + (int)(deg * 10.0f);
    if (pwm < 1100) pwm = 1100;
    if (pwm > 1900) pwm = 1900;
    Servo_Steering_Set((uint16_t)pwm);
}
