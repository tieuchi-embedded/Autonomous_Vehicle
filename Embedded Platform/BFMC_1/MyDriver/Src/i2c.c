/*
 * i2c.c
 *
 *  Created on: Dec 19, 2025
 *      Author: Admin
 */
#include "mydriver.h"

void I2C1_Master_Init()
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->MODER &= ~((0b11<<(8*2)) | (0b11<<(9*2)));
    GPIOB->MODER |=  ((0b10<<(8*2)) | (0b10<<(9*2)));  // AF
    GPIOB->OTYPER |= (1<<8) | (1<<9);                  // OD
    GPIOB->OSPEEDR |= (0b11<<(8*2)) | (0b11<<(9*2));   // High
    GPIOB->PUPDR &= ~((0b11<<(8*2)) | (0b11<<(9*2)));
    GPIOB->PUPDR |=  ((0b01<<(8*2)) | (0b01<<(9*2)));  // Pull-up

    GPIOB->AFR[1] &= ~(0xF<<(0*4));
    GPIOB->AFR[1] |= (0x4<<(0*4));
    GPIOB->AFR[1] &= ~(0xF<<(1*4));
    GPIOB->AFR[1] |= (0x4<<(1*4));

    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->CR2 = 16;         // PCLK1=16MHz
    I2C1->CCR = 80;         // 100kHz
    I2C1->TRISE = 17;       // 1000ns @16MHz
    I2C1->CR1 |= I2C_CR1_ACK;
    I2C1->CR1 |= I2C_CR1_PE;
}

void I2C1_WriteReg(uint8_t dev, uint8_t reg, uint8_t data)
{
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));

    I2C1->DR = dev << 1;                    // Write
    for (int i = 0; i < 1000; i++) {
        if (I2C1->SR1 & I2C_SR1_ADDR) {
            (void)I2C1->SR2;
            break;
        }
        if (i == 999)
            goto i2c_error;
    }

    while (!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = reg;
    while (!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = data;
    while (!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;
    return;

    i2c_error:
    I2C1->SR1 &= ~I2C_SR1_AF;
    I2C1->CR1 |= I2C_CR1_STOP;
    return;
}

void I2C1_ReadMulti(uint8_t dev, uint8_t reg, uint8_t *buf, uint8_t len)
{
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));

    I2C1->DR = dev << 1;
    for (int i = 0; i < 1000; i++) {
        if (I2C1->SR1 & I2C_SR1_ADDR) {
            (void)I2C1->SR2;
            break;
        }
        if (i == 999)
            goto i2c_error;
    }

    while (!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = reg;

    while (!(I2C1->SR1 & I2C_SR1_BTF));

    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));

    I2C1->DR = (dev << 1) | 1;
    for (int i = 0; i < 1000; i++) {
        if (I2C1->SR1 & I2C_SR1_ADDR) {
            (void)I2C1->SR2;
            break;
        }
        if (i == 999)
            goto i2c_error;
    }

    for (uint8_t i = 0; i < len; i++) {
        if (i == len-1) {
            I2C1->CR1 &= ~I2C_CR1_ACK;
            I2C1->CR1 |= I2C_CR1_STOP;
        }
        while (!(I2C1->SR1 & I2C_SR1_RXNE));
        buf[i] = I2C1->DR;
    }
    I2C1->CR1 |= I2C_CR1_ACK;
    return;

    i2c_error:
    I2C1->SR1 &= ~I2C_SR1_AF;
    I2C1->CR1 |= I2C_CR1_STOP;
    return;
}
