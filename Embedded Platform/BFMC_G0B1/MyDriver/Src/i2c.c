/*
 * i2c.c — G0B1 port
 *
 * G0B1 I2C is a v2 peripheral — fundamentally different programming model
 * from F411's v1 (SR1/SR2/DR/CCR/TRISE byte-polling with manual START/STOP).
 *
 * v2 model: configure the whole transaction in CR2 (SADD, NBYTES, RD_WRN,
 * START, AUTOEND/RELOAD, STOP), then poll TXIS (write)/RXNE (read)/TC.
 *
 * TIMINGR = 0x10805E89 — ST standard reference value for 100kHz Standard-mode
 * @ fI2CCLK = 16MHz (HSI16, no PLL). PRESC=1, SCLDEL=8, SDADEL=0, SCLH=0x5E, SCLL=0x89.
 *
 * Pin AF: PB8/PB9 = I2C1 SCL/SDA is AF6 on G0B1 (was AF4 on F411).
 */
#include "mydriver.h"

#define I2C_TIMEOUT_LOOPS  100000

static int i2c_wait_flag(volatile uint32_t *reg, uint32_t mask, uint32_t set)
{
    for (uint32_t i = 0; i < I2C_TIMEOUT_LOOPS; i++) {
        uint32_t v = *reg & mask;
        if (set ? (v == mask) : (v == 0))
            return 1;
    }
    return 0;
}

void I2C1_Master_Init(void)
{
    RCC->IOPENR  |= RCC_IOPENR_GPIOBEN;
    RCC->APBENR1 |= RCC_APBENR1_I2C1EN;

    GPIOB->MODER &= ~((0b11<<(8*2)) | (0b11<<(9*2)));
    GPIOB->MODER |=  ((0b10<<(8*2)) | (0b10<<(9*2)));  // AF
    GPIOB->OTYPER |= (1<<8) | (1<<9);                  // OD
    GPIOB->OSPEEDR |= (0b11<<(8*2)) | (0b11<<(9*2));   // High
    GPIOB->PUPDR &= ~((0b11<<(8*2)) | (0b11<<(9*2)));
    GPIOB->PUPDR |=  ((0b01<<(8*2)) | (0b01<<(9*2)));  // Pull-up

    GPIOB->AFR[1] &= ~(0xF<<(0*4));
    GPIOB->AFR[1] |= (0x6<<(0*4));   // AF6 (I2C1_SCL on PB8)
    GPIOB->AFR[1] &= ~(0xF<<(1*4));
    GPIOB->AFR[1] |= (0x6<<(1*4));   // AF6 (I2C1_SDA on PB9)

    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->TIMINGR = 0x10805E89;   // 100kHz @ 16MHz I2CCLK (HSI16, no PLL)
    I2C1->CR1 |= I2C_CR1_PE;
}

void I2C1_WriteReg(uint8_t dev, uint8_t reg, uint8_t data)
{
    /* Transaction 1: write [reg, data] (2 bytes), AUTOEND generates STOP */
    I2C1->CR2 = ((uint32_t)dev << 1)
              | (2U << I2C_CR2_NBYTES_Pos)
              | I2C_CR2_AUTOEND
              | I2C_CR2_START;

    if (!i2c_wait_flag(&I2C1->ISR, I2C_ISR_TXIS, 1)) goto i2c_error;
    I2C1->TXDR = reg;

    if (!i2c_wait_flag(&I2C1->ISR, I2C_ISR_TXIS, 1)) goto i2c_error;
    I2C1->TXDR = data;

    i2c_wait_flag(&I2C1->ISR, I2C_ISR_STOPF, 1);
    I2C1->ICR = I2C_ICR_STOPCF;
    return;

i2c_error:
    I2C1->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;
    I2C1->CR2 |= I2C_CR2_STOP;
    return;
}

void I2C1_ReadMulti(uint8_t dev, uint8_t reg, uint8_t *buf, uint8_t len)
{
    /* Transaction 1: write register address (1 byte), no STOP (restart follows) */
    I2C1->CR2 = ((uint32_t)dev << 1)
              | (1U << I2C_CR2_NBYTES_Pos)
              | I2C_CR2_START;

    if (!i2c_wait_flag(&I2C1->ISR, I2C_ISR_TXIS, 1)) goto i2c_error;
    I2C1->TXDR = reg;

    if (!i2c_wait_flag(&I2C1->ISR, I2C_ISR_TC, 1)) goto i2c_error;

    /* Transaction 2: repeated START, read `len` bytes, AUTOEND generates STOP */
    I2C1->CR2 = ((uint32_t)dev << 1)
              | I2C_CR2_RD_WRN
              | ((uint32_t)len << I2C_CR2_NBYTES_Pos)
              | I2C_CR2_AUTOEND
              | I2C_CR2_START;

    for (uint8_t i = 0; i < len; i++) {
        if (!i2c_wait_flag(&I2C1->ISR, I2C_ISR_RXNE, 1)) goto i2c_error;
        buf[i] = (uint8_t)I2C1->RXDR;
    }

    i2c_wait_flag(&I2C1->ISR, I2C_ISR_STOPF, 1);
    I2C1->ICR = I2C_ICR_STOPCF;
    return;

i2c_error:
    I2C1->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;
    I2C1->CR2 |= I2C_CR2_STOP;
    return;
}
