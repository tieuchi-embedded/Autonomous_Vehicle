/*
 * servo.c — G0B1 port
 *
 * PA0 = TIM2_CH1 (AF2 on G0B1, was AF1 on F411)
 * PA1 = TIM2_CH2 (AF2 on G0B1, was AF1 on F411)
 * RCC: GPIOA clock now under IOPENR, TIM2 clock under APBENR1 (same bit position)
 */

#include "mydriver.h"

void PWM_Init(void)
{
    RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR1 |= RCC_APBENR1_TIM2EN;

    GPIOA->MODER &= ~(0b11 << (0 * 2));  // clear mode PA0
    GPIOA->MODER |=  (0b10 << (0 * 2));  // set AF mode
    GPIOA->AFR[0] &= ~(0xF << (0 * 4));
    GPIOA->AFR[0] |=  (0x2 << (0 * 4)); // AF2 (TIM2_CH1)

    GPIOA->MODER &= ~(0b11 << (1 * 2));
    GPIOA->MODER |=  (0b10 << (1 * 2));
    GPIOA->AFR[0] &= ~(0xF << (1 * 4));
    GPIOA->AFR[0] |=  (0x2 << (1 * 4)); // AF2 (TIM2_CH2)

    TIM2->PSC = 16 - 1;     // 16MHz/16 = 1MHz tick
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
