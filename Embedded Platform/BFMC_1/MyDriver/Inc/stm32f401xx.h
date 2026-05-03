/**
 ******************************************************************************
 * @file    stm32f401xx.h
 * @brief   STM32F401xx peripheral register definitions
 *          Based on RM0368 Rev5 - STM32F401xB/C and STM32F401xD/E Reference Manual
 *
 * Target: STM32F401CCU6 / STM32F401CEU6 (Cortex-M4F, 84MHz)
 *
 * Usage:
 *   #include "stm32f401xx.h"
 *   RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;  // enable GPIOC clock
 *   GPIOC->MODER |= (1U << 26);            // set PC13 as output
 *   GPIOC->ODR   ^= (1U << 13);            // toggle PC13
 *
 * Key differences from STM32F407xx:
 *   - Max clock: 84 MHz (vs 168 MHz on F407)
 *   - No TIM6, TIM7, TIM8, TIM12, TIM13, TIM14
 *   - No DAC, no CAN1/CAN2
 *   - No UART4, UART5; only USART1/2/6
 *   - Single ADC (ADC1 only), no ADC2/ADC3
 *   - GPIO ports: A, B, C, D, E, H only (no F, G, I)
 *   - No Ethernet, no DCMI, no RNG
 *   - Flash latency: 0WS<=24MHz, 1WS<=48MHz, 2WS<=84MHz (at VDD 2.7-3.6V)
 *   - RCC: no PLLI2S PLLR divider, no AHB3
 ******************************************************************************
 */

#ifndef STM32F401XX_H
#define STM32F401XX_H

#include <stdint.h>

/*===========================================================================
 * SECTION 1 — CORE REGISTERS (Cortex-M4F, ARM-defined)
 *===========================================================================*/

/* FPU coprocessor access control register — ARM TRM */
#define SCB_CPACR       (*((volatile uint32_t *)0xE000ED88U))

/* SysTick — 24-bit countdown timer built into every Cortex-M */
typedef struct {
    volatile uint32_t CTRL;     /* Control and status         */
    volatile uint32_t LOAD;     /* Reload value               */
    volatile uint32_t VAL;      /* Current value              */
    volatile uint32_t CALIB;    /* Calibration (read-only)    */
} SysTick_TypeDef;

#define SYSTICK_BASE    0xE000E010U
#define SYSTICK         ((SysTick_TypeDef *) SYSTICK_BASE)

/* NVIC — Nested Vectored Interrupt Controller */
typedef struct {
    volatile uint32_t ISER[8];  /* Interrupt set-enable       0x000 */
    uint32_t          RESERVED0[24];
    volatile uint32_t ICER[8];  /* Interrupt clear-enable     0x080 */
    uint32_t          RESERVED1[24];
    volatile uint32_t ISPR[8];  /* Interrupt set-pending      0x100 */
    uint32_t          RESERVED2[24];
    volatile uint32_t ICPR[8];  /* Interrupt clear-pending    0x180 */
    uint32_t          RESERVED3[24];
    volatile uint32_t IABR[8];  /* Interrupt active-bit       0x200 */
    uint32_t          RESERVED4[56];
    volatile uint8_t  IP[240];  /* Interrupt priority (8-bit) 0x300 */
    uint32_t          RESERVED5[644];
    volatile uint32_t STIR;     /* Software trigger interrupt 0xE00 */
} NVIC_TypeDef;

#define NVIC_BASE       0xE000E100U
#define NVIC            ((NVIC_TypeDef *) NVIC_BASE)

/* SCB — System Control Block */
typedef struct {
    volatile uint32_t CPUID;    /* CPUID base register        */
    volatile uint32_t ICSR;     /* Interrupt control/state    */
    volatile uint32_t VTOR;     /* Vector table offset        */
    volatile uint32_t AIRCR;    /* App interrupt/reset ctrl   */
    volatile uint32_t SCR;      /* System control             */
    volatile uint32_t CCR;      /* Configuration and control  */
    volatile uint8_t  SHP[12];  /* System handler priority    */
    volatile uint32_t SHCSR;    /* System handler ctrl/status */
    volatile uint32_t CFSR;     /* Configurable fault status  */
    volatile uint32_t HFSR;     /* HardFault status           */
    volatile uint32_t DFSR;     /* Debug fault status         */
    volatile uint32_t MMFAR;    /* MemManage fault address    */
    volatile uint32_t BFAR;     /* BusFault address           */
    volatile uint32_t AFSR;     /* Auxiliary fault status     */
} SCB_TypeDef;

#define SCB_BASE        0xE000ED00U
#define SCB             ((SCB_TypeDef *) SCB_BASE)

/*===========================================================================
 * SECTION 2 — BUS BASE ADDRESSES (RM0368 Table 1, memory map)
 *===========================================================================*/

#define PERIPH_BASE         0x40000000U     /* Peripheral base              */

#define APB1_BASE           (PERIPH_BASE + 0x00000000U)   /* APB1 bus       */
#define APB2_BASE           (PERIPH_BASE + 0x00010000U)   /* APB2 bus       */
#define AHB1_BASE           (PERIPH_BASE + 0x00020000U)   /* AHB1 bus       */
#define AHB2_BASE           (PERIPH_BASE + 0x10000000U)   /* AHB2 bus       */

/*===========================================================================
 * SECTION 3 — PERIPHERAL BASE ADDRESSES (RM0368 Table 1)
 *
 * NOTE: F401 does NOT have: TIM6, TIM7, TIM8, TIM12, TIM13, TIM14,
 *       DAC, CAN1, CAN2, UART4, UART5, ADC2, ADC3, GPIOF, GPIOG, GPIOI,
 *       Ethernet, DCMI, RNG, FSMC.
 *===========================================================================*/

/* --- APB1 peripherals --- */
#define TIM2_BASE           (APB1_BASE + 0x0000U)
#define TIM3_BASE           (APB1_BASE + 0x0400U)
#define TIM4_BASE           (APB1_BASE + 0x0800U)
#define TIM5_BASE           (APB1_BASE + 0x0C00U)
#define RTC_BASE            (APB1_BASE + 0x2800U)
#define WWDG_BASE           (APB1_BASE + 0x2C00U)
#define IWDG_BASE           (APB1_BASE + 0x3000U)
#define SPI2_BASE           (APB1_BASE + 0x3800U)
#define SPI3_BASE           (APB1_BASE + 0x3C00U)
#define USART2_BASE         (APB1_BASE + 0x4400U)
#define I2C1_BASE           (APB1_BASE + 0x5400U)
#define I2C2_BASE           (APB1_BASE + 0x5800U)
#define I2C3_BASE           (APB1_BASE + 0x5C00U)
#define PWR_BASE            (APB1_BASE + 0x7000U)

/* --- APB2 peripherals --- */
#define TIM1_BASE           (APB2_BASE + 0x0000U)
#define USART1_BASE         (APB2_BASE + 0x1000U)
#define USART6_BASE         (APB2_BASE + 0x1400U)
#define ADC1_BASE           (APB2_BASE + 0x2000U)
#define ADC_COMMON_BASE     (APB2_BASE + 0x2300U)
#define SDIO_BASE           (APB2_BASE + 0x2C00U)
#define SPI1_BASE           (APB2_BASE + 0x3000U)
#define SPI4_BASE           (APB2_BASE + 0x3400U)   /* F401xC/xE only     */
#define SYSCFG_BASE         (APB2_BASE + 0x3800U)
#define EXTI_BASE           (APB2_BASE + 0x3C00U)
#define TIM9_BASE           (APB2_BASE + 0x4000U)
#define TIM10_BASE          (APB2_BASE + 0x4400U)
#define TIM11_BASE          (APB2_BASE + 0x4800U)

/* --- AHB1 peripherals --- */
#define GPIOA_BASE          (AHB1_BASE + 0x0000U)
#define GPIOB_BASE          (AHB1_BASE + 0x0400U)
#define GPIOC_BASE          (AHB1_BASE + 0x0800U)
#define GPIOD_BASE          (AHB1_BASE + 0x0C00U)
#define GPIOE_BASE          (AHB1_BASE + 0x1000U)
#define GPIOH_BASE          (AHB1_BASE + 0x1C00U)   /* Only PA, PB, PC, PD, PE, PH */
#define CRC_BASE            (AHB1_BASE + 0x3000U)
#define RCC_BASE            (AHB1_BASE + 0x3800U)
#define FLASH_BASE          (AHB1_BASE + 0x3C00U)
#define DMA1_BASE           (AHB1_BASE + 0x6000U)
#define DMA2_BASE           (AHB1_BASE + 0x6400U)

/* --- AHB2 peripherals --- */
#define USB_OTG_FS_BASE     (AHB2_BASE + 0x0000U)

/*===========================================================================
 * SECTION 4 — GPIO (RM0368 Chapter 8)
 *
 * Same register layout as F407.
 * F401 has ports: A, B, C, D, E, H (no F, G, I)
 *===========================================================================*/

typedef struct {
    volatile uint32_t MODER;    /* Mode register              offset 0x00 */
    volatile uint32_t OTYPER;   /* Output type register       offset 0x04 */
    volatile uint32_t OSPEEDR;  /* Output speed register      offset 0x08 */
    volatile uint32_t PUPDR;    /* Pull-up/pull-down register offset 0x0C */
    volatile uint32_t IDR;      /* Input data register        offset 0x10 */
    volatile uint32_t ODR;      /* Output data register       offset 0x14 */
    volatile uint32_t BSRR;     /* Bit set/reset register     offset 0x18 */
    volatile uint32_t LCKR;     /* Configuration lock         offset 0x1C */
    volatile uint32_t AFR[2];   /* Alternate function [0]=low,[1]=high    */
} GPIO_TypeDef;

#define GPIOA   ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB   ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC   ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD   ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE   ((GPIO_TypeDef *) GPIOE_BASE)
#define GPIOH   ((GPIO_TypeDef *) GPIOH_BASE)

/*===========================================================================
 * SECTION 5 — RCC (Reset and Clock Control) (RM0368 Chapter 6)
 *
 * Clock tree summary (F401):
 *   HSI  → 16 MHz RC
 *   HSE  → 4–26 MHz external crystal
 *   PLL  → (HSI/M or HSE/M) × N / P  (max 84 MHz SYSCLK)
 *          × N / Q → USB OTG FS (48 MHz required)
 *   AHB  → SYSCLK / HPRE        (max 84 MHz)
 *   APB1 → AHB / PPRE1          (max 42 MHz)
 *   APB2 → AHB / PPRE2          (max 84 MHz)
 *
 * NOTE: F401 RCC has no AHB2RSTR, AHB3RSTR, AHB3ENR, AHB3LPENR.
 *       PLLI2S is present (for I2S audio), PLLSAI is NOT present.
 *===========================================================================*/

typedef struct {
    volatile uint32_t CR;           /* Clock control              offset 0x00 */
    volatile uint32_t PLLCFGR;      /* PLL configuration          offset 0x04 */
    volatile uint32_t CFGR;         /* Clock configuration        offset 0x08 */
    volatile uint32_t CIR;          /* Clock interrupt            offset 0x0C */
    volatile uint32_t AHB1RSTR;     /* AHB1 peripheral reset      offset 0x10 */
    volatile uint32_t AHB2RSTR;     /* AHB2 peripheral reset      offset 0x14 */
    uint32_t          RESERVED0[2]; /*                            offset 0x18 */
    volatile uint32_t APB1RSTR;     /* APB1 peripheral reset      offset 0x20 */
    volatile uint32_t APB2RSTR;     /* APB2 peripheral reset      offset 0x24 */
    uint32_t          RESERVED1[2]; /*                            offset 0x28 */
    volatile uint32_t AHB1ENR;      /* AHB1 clock enable          offset 0x30 */
    volatile uint32_t AHB2ENR;      /* AHB2 clock enable          offset 0x34 */
    uint32_t          RESERVED2[2]; /*                            offset 0x38 */
    volatile uint32_t APB1ENR;      /* APB1 clock enable          offset 0x40 */
    volatile uint32_t APB2ENR;      /* APB2 clock enable          offset 0x44 */
    uint32_t          RESERVED3[2]; /*                            offset 0x48 */
    volatile uint32_t AHB1LPENR;    /* AHB1 clock enable low-power offset 0x50 */
    volatile uint32_t AHB2LPENR;    /* AHB2 clock enable low-power offset 0x54 */
    uint32_t          RESERVED4[2]; /*                            offset 0x58 */
    volatile uint32_t APB1LPENR;    /* APB1 clock enable low-power offset 0x60 */
    volatile uint32_t APB2LPENR;    /* APB2 clock enable low-power offset 0x64 */
    uint32_t          RESERVED5[2]; /*                            offset 0x68 */
    volatile uint32_t BDCR;         /* Backup domain control      offset 0x70 */
    volatile uint32_t CSR;          /* Clock control and status   offset 0x74 */
    uint32_t          RESERVED6[2]; /*                            offset 0x78 */
    volatile uint32_t SSCGR;        /* Spread spectrum clock gen  offset 0x80 */
    volatile uint32_t PLLI2SCFGR;   /* PLLI2S configuration       offset 0x84 */
    uint32_t          RESERVED7;    /*                            offset 0x88 */
    volatile uint32_t DCKCFGR;      /* Dedicated clocks config    offset 0x8C */
} RCC_TypeDef;

#define RCC     ((RCC_TypeDef *) RCC_BASE)

/* RCC AHB1ENR bit positions */
#define RCC_AHB1ENR_GPIOAEN     (1U << 0)
#define RCC_AHB1ENR_GPIOBEN     (1U << 1)
#define RCC_AHB1ENR_GPIOCEN     (1U << 2)
#define RCC_AHB1ENR_GPIODEN     (1U << 3)
#define RCC_AHB1ENR_GPIOEEN     (1U << 4)
#define RCC_AHB1ENR_GPIOHEN     (1U << 7)
#define RCC_AHB1ENR_CRCEN       (1U << 12)
#define RCC_AHB1ENR_DMA1EN      (1U << 21)
#define RCC_AHB1ENR_DMA2EN      (1U << 22)

/* RCC APB1ENR bit positions */
#define RCC_APB1ENR_TIM2EN      (1U << 0)
#define RCC_APB1ENR_TIM3EN      (1U << 1)
#define RCC_APB1ENR_TIM4EN      (1U << 2)
#define RCC_APB1ENR_TIM5EN      (1U << 3)
#define RCC_APB1ENR_SPI2EN      (1U << 14)
#define RCC_APB1ENR_SPI3EN      (1U << 15)
#define RCC_APB1ENR_USART2EN    (1U << 17)
#define RCC_APB1ENR_I2C1EN      (1U << 21)
#define RCC_APB1ENR_I2C2EN      (1U << 22)
#define RCC_APB1ENR_I2C3EN      (1U << 23)
#define RCC_APB1ENR_PWREN       (1U << 28)

/* RCC APB2ENR bit positions */
#define RCC_APB2ENR_TIM1EN      (1U << 0)
#define RCC_APB2ENR_USART1EN    (1U << 4)
#define RCC_APB2ENR_USART6EN    (1U << 5)
#define RCC_APB2ENR_ADC1EN      (1U << 8)
#define RCC_APB2ENR_SDIOEN      (1U << 11)
#define RCC_APB2ENR_SPI1EN      (1U << 12)
#define RCC_APB2ENR_SPI4EN      (1U << 13)
#define RCC_APB2ENR_SYSCFGEN    (1U << 14)
#define RCC_APB2ENR_TIM9EN      (1U << 16)
#define RCC_APB2ENR_TIM10EN     (1U << 17)
#define RCC_APB2ENR_TIM11EN     (1U << 18)

/* RCC CR bit positions */
#define RCC_CR_HSION            (1U << 0)   /* HSI oscillator ON          */
#define RCC_CR_HSIRDY           (1U << 1)   /* HSI ready                  */
#define RCC_CR_HSEON            (1U << 16)  /* HSE oscillator ON          */
#define RCC_CR_HSERDY           (1U << 17)  /* HSE ready                  */
#define RCC_CR_HSEBYP           (1U << 18)  /* HSE bypass                 */
#define RCC_CR_CSSON            (1U << 19)  /* Clock security system ON   */
#define RCC_CR_PLLON            (1U << 24)  /* PLL ON                     */
#define RCC_CR_PLLRDY           (1U << 25)  /* PLL ready                  */

/* RCC CFGR bit positions */
#define RCC_CFGR_SW_HSI         (0U << 0)   /* System clock = HSI         */
#define RCC_CFGR_SW_HSE         (1U << 0)   /* System clock = HSE         */
#define RCC_CFGR_SW_PLL         (2U << 0)   /* System clock = PLL         */
#define RCC_CFGR_SWS_MASK       (3U << 2)   /* Clock switch status mask   */
#define RCC_CFGR_SWS_PLL        (2U << 2)   /* PLL used as system clock   */

/*
 * PLLCFGR: PLL configuration (RM0368 §6.3.2)
 *   PLLM [5:0]  — VCO input divider   (2..63,  typ 8 for 1MHz VCO input)
 *   PLLN [14:6] — VCO multiplier      (192..432)
 *   PLLP [17:16]— SYSCLK divider      (00=÷2, 01=÷4, 10=÷6, 11=÷8)
 *   PLLSRC [22] — 0=HSI, 1=HSE
 *   PLLQ [27:24]— USB/SDIO divider    (2..15, must produce 48 MHz)
 *
 * Example: HSE=25MHz → PLLM=25, PLLN=336, PLLP=÷4 → SYSCLK=84MHz
 *                                          PLLQ=7  → USB=48MHz
 */
#define RCC_PLLCFGR_PLLSRC_HSI (0U << 22)
#define RCC_PLLCFGR_PLLSRC_HSE (1U << 22)

/*===========================================================================
 * SECTION 6 — USART / UART (RM0368 Chapter 19)
 *
 * F401 has: USART1 (APB2), USART2 (APB1), USART6 (APB2)
 * No UART4, UART5 on F401.
 * BRR = fCLK / baud  (for OVER8=0, which is default)
 *===========================================================================*/

typedef struct {
    volatile uint32_t SR;       /* Status register            offset 0x00 */
    volatile uint32_t DR;       /* Data register              offset 0x04 */
    volatile uint32_t BRR;      /* Baud rate register         offset 0x08 */
    volatile uint32_t CR1;      /* Control register 1         offset 0x0C */
    volatile uint32_t CR2;      /* Control register 2         offset 0x10 */
    volatile uint32_t CR3;      /* Control register 3         offset 0x14 */
    volatile uint32_t GTPR;     /* Guard time and prescaler   offset 0x18 */
} USART_TypeDef;

#define USART1  ((USART_TypeDef *) USART1_BASE)
#define USART2  ((USART_TypeDef *) USART2_BASE)
#define USART6  ((USART_TypeDef *) USART6_BASE)

/* USART SR bit positions */
#define USART_SR_PE             (1U << 0)   /* Parity error               */
#define USART_SR_FE             (1U << 1)   /* Framing error              */
#define USART_SR_NF             (1U << 2)   /* Noise detected             */
#define USART_SR_ORE            (1U << 3)   /* Overrun error              */
#define USART_SR_IDLE           (1U << 4)   /* Idle line detected         */
#define USART_SR_RXNE           (1U << 5)   /* Read data register not empty */
#define USART_SR_TC             (1U << 6)   /* Transmission complete      */
#define USART_SR_TXE            (1U << 7)   /* Transmit data register empty */

/* USART CR1 bit positions */
#define USART_CR1_SBK           (1U << 0)   /* Send break                 */
#define USART_CR1_RE            (1U << 2)   /* Receiver enable            */
#define USART_CR1_TE            (1U << 3)   /* Transmitter enable         */
#define USART_CR1_IDLEIE        (1U << 4)   /* IDLE interrupt enable      */
#define USART_CR1_RXNEIE        (1U << 5)   /* RXNE interrupt enable      */
#define USART_CR1_TCIE          (1U << 6)   /* TX complete interrupt      */
#define USART_CR1_TXEIE         (1U << 7)   /* TXE interrupt enable       */
#define USART_CR1_PS            (1U << 9)   /* Parity selection           */
#define USART_CR1_PCE           (1U << 10)  /* Parity control enable      */
#define USART_CR1_M             (1U << 12)  /* Word length (0=8bit,1=9bit)*/
#define USART_CR1_UE            (1U << 13)  /* USART enable               */

/*===========================================================================
 * SECTION 7 — SPI (RM0368 Chapter 20)
 *
 * F401 has: SPI1 (APB2), SPI2 (APB1), SPI3 (APB1), SPI4 (APB2, xC/xE only)
 *===========================================================================*/

typedef struct {
    volatile uint32_t CR1;      /* Control register 1         offset 0x00 */
    volatile uint32_t CR2;      /* Control register 2         offset 0x04 */
    volatile uint32_t SR;       /* Status register            offset 0x08 */
    volatile uint32_t DR;       /* Data register              offset 0x0C */
    volatile uint32_t CRCPR;    /* CRC polynomial             offset 0x10 */
    volatile uint32_t RXCRCR;   /* RX CRC register            offset 0x14 */
    volatile uint32_t TXCRCR;   /* TX CRC register            offset 0x18 */
    volatile uint32_t I2SCFGR;  /* I2S configuration          offset 0x1C */
    volatile uint32_t I2SPR;    /* I2S prescaler              offset 0x20 */
} SPI_TypeDef;

#define SPI1    ((SPI_TypeDef *) SPI1_BASE)
#define SPI2    ((SPI_TypeDef *) SPI2_BASE)
#define SPI3    ((SPI_TypeDef *) SPI3_BASE)
#define SPI4    ((SPI_TypeDef *) SPI4_BASE)   /* F401xC/xE only */

/* SPI CR1 bit positions */
#define SPI_CR1_CPHA            (1U << 0)   /* Clock phase                */
#define SPI_CR1_CPOL            (1U << 1)   /* Clock polarity             */
#define SPI_CR1_MSTR            (1U << 2)   /* Master selection           */
#define SPI_CR1_BR_DIV2         (0U << 3)   /* Baud rate = fPCLK/2        */
#define SPI_CR1_BR_DIV4         (1U << 3)   /* Baud rate = fPCLK/4        */
#define SPI_CR1_BR_DIV8         (2U << 3)   /* Baud rate = fPCLK/8        */
#define SPI_CR1_BR_DIV16        (3U << 3)   /* Baud rate = fPCLK/16       */
#define SPI_CR1_BR_DIV32        (4U << 3)   /* Baud rate = fPCLK/32       */
#define SPI_CR1_BR_DIV64        (5U << 3)   /* Baud rate = fPCLK/64       */
#define SPI_CR1_BR_DIV128       (6U << 3)   /* Baud rate = fPCLK/128      */
#define SPI_CR1_BR_DIV256       (7U << 3)   /* Baud rate = fPCLK/256      */
#define SPI_CR1_SPE             (1U << 6)   /* SPI enable                 */
#define SPI_CR1_SSI             (1U << 8)   /* Internal slave select      */
#define SPI_CR1_SSM             (1U << 9)   /* Software slave management  */
#define SPI_CR1_DFF             (1U << 11)  /* Data frame format 0=8b,1=16b*/

/* SPI SR bit positions */
#define SPI_SR_RXNE             (1U << 0)   /* Receive buffer not empty   */
#define SPI_SR_TXE              (1U << 1)   /* Transmit buffer empty      */
#define SPI_SR_BSY              (1U << 7)   /* Busy flag                  */

/*===========================================================================
 * SECTION 8 — I2C (RM0368 Chapter 18)
 *
 * F401 has: I2C1, I2C2, I2C3
 *===========================================================================*/

typedef struct {
    volatile uint32_t CR1;      /* Control register 1         offset 0x00 */
    volatile uint32_t CR2;      /* Control register 2         offset 0x04 */
    volatile uint32_t OAR1;     /* Own address register 1     offset 0x08 */
    volatile uint32_t OAR2;     /* Own address register 2     offset 0x0C */
    volatile uint32_t DR;       /* Data register              offset 0x10 */
    volatile uint32_t SR1;      /* Status register 1          offset 0x14 */
    volatile uint32_t SR2;      /* Status register 2          offset 0x18 */
    volatile uint32_t CCR;      /* Clock control              offset 0x1C */
    volatile uint32_t TRISE;    /* Rise time                  offset 0x20 */
    volatile uint32_t FLTR;     /* Digital noise filter       offset 0x24 */
} I2C_TypeDef;

#define I2C1    ((I2C_TypeDef *) I2C1_BASE)
#define I2C2    ((I2C_TypeDef *) I2C2_BASE)
#define I2C3    ((I2C_TypeDef *) I2C3_BASE)

/* I2C CR1 bit positions */
#define I2C_CR1_PE              (1U << 0)   /* Peripheral enable          */
#define I2C_CR1_START           (1U << 8)   /* Start generation           */
#define I2C_CR1_STOP            (1U << 9)   /* Stop generation            */
#define I2C_CR1_ACK             (1U << 10)  /* Acknowledge enable         */
#define I2C_CR1_SWRST           (1U << 15)  /* Software reset             */

/* I2C SR1 bit positions */
#define I2C_SR1_SB              (1U << 0)   /* Start bit                  */
#define I2C_SR1_ADDR            (1U << 1)   /* Address sent/matched       */
#define I2C_SR1_BTF             (1U << 2)   /* Byte transfer finished     */
#define I2C_SR1_RXNE            (1U << 6)   /* Data register not empty    */
#define I2C_SR1_TXE             (1U << 7)   /* Data register empty        */
#define I2C_SR1_AF              (1U << 10)  /* Acknowledge failure        */

/*===========================================================================
 * SECTION 9 — TIMERS (RM0368 Chapter 11-14)
 *
 * F401 timers:
 *   TIM1          : Advanced-control (APB2), 16-bit, 4 CC channels, BRK/DTG
 *   TIM2          : General-purpose (APB1), 32-bit, 4 CC channels
 *   TIM3, TIM4    : General-purpose (APB1), 16-bit, 4 CC channels
 *   TIM5          : General-purpose (APB1), 32-bit, 4 CC channels
 *   TIM9          : General-purpose (APB2), 16-bit, 2 CC channels
 *   TIM10, TIM11  : General-purpose (APB2), 16-bit, 1 CC channel
 *
 * NOTE: No TIM6, TIM7, TIM8, TIM12, TIM13, TIM14 on F401.
 *===========================================================================*/

typedef struct {
    volatile uint32_t CR1;      /* Control register 1         offset 0x00 */
    volatile uint32_t CR2;      /* Control register 2         offset 0x04 */
    volatile uint32_t SMCR;     /* Slave mode control         offset 0x08 */
    volatile uint32_t DIER;     /* DMA/interrupt enable       offset 0x0C */
    volatile uint32_t SR;       /* Status register            offset 0x10 */
    volatile uint32_t EGR;      /* Event generation           offset 0x14 */
    volatile uint32_t CCMR1;    /* Capture/compare mode 1     offset 0x18 */
    volatile uint32_t CCMR2;    /* Capture/compare mode 2     offset 0x1C */
    volatile uint32_t CCER;     /* Capture/compare enable     offset 0x20 */
    volatile uint32_t CNT;      /* Counter                    offset 0x24 */
    volatile uint32_t PSC;      /* Prescaler                  offset 0x28 */
    volatile uint32_t ARR;      /* Auto-reload register       offset 0x2C */
    volatile uint32_t RCR;      /* Repetition counter (TIM1)  offset 0x30 */
    volatile uint32_t CCR1;     /* Capture/compare register 1 offset 0x34 */
    volatile uint32_t CCR2;     /* Capture/compare register 2 offset 0x38 */
    volatile uint32_t CCR3;     /* Capture/compare register 3 offset 0x3C */
    volatile uint32_t CCR4;     /* Capture/compare register 4 offset 0x40 */
    volatile uint32_t BDTR;     /* Break and dead-time (TIM1) offset 0x44 */
    volatile uint32_t DCR;      /* DMA control                offset 0x48 */
    volatile uint32_t DMAR;     /* DMA address for transfer   offset 0x4C */
    volatile uint32_t OR;       /* Timer option               offset 0x50 */
} TIM_TypeDef;

#define TIM1    ((TIM_TypeDef *) TIM1_BASE)
#define TIM2    ((TIM_TypeDef *) TIM2_BASE)
#define TIM3    ((TIM_TypeDef *) TIM3_BASE)
#define TIM4    ((TIM_TypeDef *) TIM4_BASE)
#define TIM5    ((TIM_TypeDef *) TIM5_BASE)
#define TIM9    ((TIM_TypeDef *) TIM9_BASE)
#define TIM10   ((TIM_TypeDef *) TIM10_BASE)
#define TIM11   ((TIM_TypeDef *) TIM11_BASE)

/* TIM CR1 bit positions */
#define TIM_CR1_CEN             (1U << 0)   /* Counter enable             */
#define TIM_CR1_UDIS            (1U << 1)   /* Update disable             */
#define TIM_CR1_URS             (1U << 2)   /* Update request source      */
#define TIM_CR1_OPM             (1U << 3)   /* One-pulse mode             */
#define TIM_CR1_DIR             (1U << 4)   /* Direction (0=up, 1=down)   */
#define TIM_CR1_ARPE            (1U << 7)   /* Auto-reload preload enable */

/* TIM DIER bit positions */
#define TIM_DIER_UIE            (1U << 0)   /* Update interrupt enable    */
#define TIM_DIER_CC1IE          (1U << 1)   /* CC1 interrupt enable       */
#define TIM_DIER_CC2IE          (1U << 2)   /* CC2 interrupt enable       */
#define TIM_DIER_CC3IE          (1U << 3)   /* CC3 interrupt enable       */
#define TIM_DIER_CC4IE          (1U << 4)   /* CC4 interrupt enable       */

/* TIM SR bit positions */
#define TIM_SR_UIF              (1U << 0)   /* Update interrupt flag      */
#define TIM_SR_CC1IF            (1U << 1)   /* CC1 interrupt flag         */
#define TIM_SR_CC2IF            (1U << 2)   /* CC2 interrupt flag         */

/* TIM BDTR — break and dead-time (TIM1 only) */
#define TIM_BDTR_MOE            (1U << 15)  /* Main output enable         */

/*===========================================================================
 * SECTION 10 — ADC (RM0368 Chapter 11)
 *
 * F401 has only ADC1 (no ADC2, no ADC3).
 * 12-bit SAR ADC, up to 2.4 MSPS.
 * ADC clock = APB2 / prescaler (max 36 MHz on F401).
 *===========================================================================*/

typedef struct {
    volatile uint32_t SR;       /* Status register            offset 0x00 */
    volatile uint32_t CR1;      /* Control register 1         offset 0x04 */
    volatile uint32_t CR2;      /* Control register 2         offset 0x08 */
    volatile uint32_t SMPR1;    /* Sample time register 1     offset 0x0C */
    volatile uint32_t SMPR2;    /* Sample time register 2     offset 0x10 */
    volatile uint32_t JOFR1;    /* Injected ch data offset 1  offset 0x14 */
    volatile uint32_t JOFR2;    /* Injected ch data offset 2  offset 0x18 */
    volatile uint32_t JOFR3;    /* Injected ch data offset 3  offset 0x1C */
    volatile uint32_t JOFR4;    /* Injected ch data offset 4  offset 0x20 */
    volatile uint32_t HTR;      /* Watchdog higher threshold  offset 0x24 */
    volatile uint32_t LTR;      /* Watchdog lower threshold   offset 0x28 */
    volatile uint32_t SQR1;     /* Regular sequence 1         offset 0x2C */
    volatile uint32_t SQR2;     /* Regular sequence 2         offset 0x30 */
    volatile uint32_t SQR3;     /* Regular sequence 3         offset 0x34 */
    volatile uint32_t JSQR;     /* Injected sequence          offset 0x38 */
    volatile uint32_t JDR1;     /* Injected data register 1   offset 0x3C */
    volatile uint32_t JDR2;     /* Injected data register 2   offset 0x40 */
    volatile uint32_t JDR3;     /* Injected data register 3   offset 0x44 */
    volatile uint32_t JDR4;     /* Injected data register 4   offset 0x48 */
    volatile uint32_t DR;       /* Regular data register      offset 0x4C */
} ADC_TypeDef;

/* ADC Common registers */
typedef struct {
    volatile uint32_t CSR;      /* Common status              offset 0x00 */
    volatile uint32_t CCR;      /* Common control             offset 0x04 */
    volatile uint32_t CDR;      /* Common data (dual mode)    offset 0x08 */
} ADC_Common_TypeDef;

#define ADC1        ((ADC_TypeDef *)        ADC1_BASE)
#define ADC_COMMON  ((ADC_Common_TypeDef *) ADC_COMMON_BASE)

/* ADC SR bit positions */
#define ADC_SR_EOC              (1U << 1)   /* End of conversion          */
#define ADC_SR_STRT             (1U << 4)   /* Regular channel start flag */

/* ADC CR1 bit positions */
#define ADC_CR1_EOCIE           (1U << 5)   /* EOC interrupt enable       */
#define ADC_CR1_RES_12BIT       (0U << 24)  /* 12-bit resolution          */
#define ADC_CR1_RES_10BIT       (1U << 24)  /* 10-bit resolution          */
#define ADC_CR1_RES_8BIT        (2U << 24)  /* 8-bit resolution           */
#define ADC_CR1_RES_6BIT        (3U << 24)  /* 6-bit resolution           */

/* ADC CR2 bit positions */
#define ADC_CR2_ADON            (1U << 0)   /* ADC ON                     */
#define ADC_CR2_CONT            (1U << 1)   /* Continuous conversion mode */
#define ADC_CR2_SWSTART         (1U << 30)  /* Start regular conversion   */

/*===========================================================================
 * SECTION 11 — DMA (RM0368 Chapter 9)
 *
 * F401 uses stream-based DMA (same as F407).
 * DMA1: 8 streams, DMA2: 8 streams.
 *===========================================================================*/

/* DMA Stream registers */
typedef struct {
    volatile uint32_t CR;       /* Stream configuration       offset 0x00 */
    volatile uint32_t NDTR;     /* Number of data             offset 0x04 */
    volatile uint32_t PAR;      /* Peripheral address         offset 0x08 */
    volatile uint32_t M0AR;     /* Memory 0 address           offset 0x0C */
    volatile uint32_t M1AR;     /* Memory 1 address           offset 0x10 */
    volatile uint32_t FCR;      /* FIFO control               offset 0x14 */
} DMA_Stream_TypeDef;

/* DMA controller registers */
typedef struct {
    volatile uint32_t LISR;     /* Low interrupt status       offset 0x00 */
    volatile uint32_t HISR;     /* High interrupt status      offset 0x04 */
    volatile uint32_t LIFCR;    /* Low interrupt flag clear   offset 0x08 */
    volatile uint32_t HIFCR;    /* High interrupt flag clear  offset 0x0C */
} DMA_TypeDef;

#define DMA1            ((DMA_TypeDef *) DMA1_BASE)
#define DMA2            ((DMA_TypeDef *) DMA2_BASE)

#define DMA1_Stream0    ((DMA_Stream_TypeDef *)(DMA1_BASE + 0x010U))
#define DMA1_Stream1    ((DMA_Stream_TypeDef *)(DMA1_BASE + 0x028U))
#define DMA1_Stream2    ((DMA_Stream_TypeDef *)(DMA1_BASE + 0x040U))
#define DMA1_Stream3    ((DMA_Stream_TypeDef *)(DMA1_BASE + 0x058U))
#define DMA1_Stream4    ((DMA_Stream_TypeDef *)(DMA1_BASE + 0x070U))
#define DMA1_Stream5    ((DMA_Stream_TypeDef *)(DMA1_BASE + 0x088U))
#define DMA1_Stream6    ((DMA_Stream_TypeDef *)(DMA1_BASE + 0x0A0U))
#define DMA1_Stream7    ((DMA_Stream_TypeDef *)(DMA1_BASE + 0x0B8U))

#define DMA2_Stream0    ((DMA_Stream_TypeDef *)(DMA2_BASE + 0x010U))
#define DMA2_Stream1    ((DMA_Stream_TypeDef *)(DMA2_BASE + 0x028U))
#define DMA2_Stream2    ((DMA_Stream_TypeDef *)(DMA2_BASE + 0x040U))
#define DMA2_Stream3    ((DMA_Stream_TypeDef *)(DMA2_BASE + 0x058U))
#define DMA2_Stream4    ((DMA_Stream_TypeDef *)(DMA2_BASE + 0x070U))
#define DMA2_Stream5    ((DMA_Stream_TypeDef *)(DMA2_BASE + 0x088U))
#define DMA2_Stream6    ((DMA_Stream_TypeDef *)(DMA2_BASE + 0x0A0U))
#define DMA2_Stream7    ((DMA_Stream_TypeDef *)(DMA2_BASE + 0x0B8U))

/* DMA Stream CR bit positions */
#define DMA_SCR_EN              (1U << 0)   /* Stream enable              */
#define DMA_SCR_DMEIE           (1U << 1)   /* Direct mode error IE       */
#define DMA_SCR_TEIE            (1U << 2)   /* Transfer error IE          */
#define DMA_SCR_HTIE            (1U << 3)   /* Half transfer IE           */
#define DMA_SCR_TCIE            (1U << 4)   /* Transfer complete IE       */
#define DMA_SCR_DIR_P2M         (0U << 6)   /* Peripheral to memory       */
#define DMA_SCR_DIR_M2P         (1U << 6)   /* Memory to peripheral       */
#define DMA_SCR_DIR_M2M         (2U << 6)   /* Memory to memory           */
#define DMA_SCR_CIRC            (1U << 8)   /* Circular mode              */
#define DMA_SCR_PINC            (1U << 9)   /* Peripheral increment mode  */
#define DMA_SCR_MINC            (1U << 10)  /* Memory increment mode      */

/*===========================================================================
 * SECTION 12 — EXTI (External Interrupt) (RM0368 Chapter 10)
 *===========================================================================*/

typedef struct {
    volatile uint32_t IMR;      /* Interrupt mask             offset 0x00 */
    volatile uint32_t EMR;      /* Event mask                 offset 0x04 */
    volatile uint32_t RTSR;     /* Rising trigger selection   offset 0x08 */
    volatile uint32_t FTSR;     /* Falling trigger selection  offset 0x0C */
    volatile uint32_t SWIER;    /* Software interrupt event   offset 0x10 */
    volatile uint32_t PR;       /* Pending register           offset 0x14 */
} EXTI_TypeDef;

#define EXTI    ((EXTI_TypeDef *) EXTI_BASE)

/*===========================================================================
 * SECTION 13 — SYSCFG (RM0368 Chapter 7)
 *
 * Used to route EXTI lines to specific GPIO ports.
 * F401: SYSCFG replaces F1xx AFIO for EXTI routing.
 *===========================================================================*/

typedef struct {
    volatile uint32_t MEMRMP;   /* Memory remap               offset 0x00 */
    volatile uint32_t PMC;      /* Peripheral mode config     offset 0x04 */
    volatile uint32_t EXTICR[4];/* External interrupt config  offset 0x08 */
    uint32_t          RESERVED[2];
    volatile uint32_t CMPCR;    /* Compensation cell control  offset 0x20 */
} SYSCFG_TypeDef;

#define SYSCFG  ((SYSCFG_TypeDef *) SYSCFG_BASE)

/*===========================================================================
 * SECTION 14 — PWR (Power Control) (RM0368 Chapter 5)
 *===========================================================================*/

typedef struct {
    volatile uint32_t CR;       /* Power control register     offset 0x00 */
    volatile uint32_t CSR;      /* Power control/status       offset 0x04 */
} PWR_TypeDef;

#define PWR     ((PWR_TypeDef *) PWR_BASE)

/*===========================================================================
 * SECTION 15 — FLASH Interface (RM0368 Chapter 3)
 *
 * Flash wait states (VDD 2.7–3.6V):
 *   0 WS : SYSCLK ≤ 24 MHz
 *   1 WS : SYSCLK ≤ 48 MHz
 *   2 WS : SYSCLK ≤ 84 MHz  ← required at 84 MHz
 *
 * Always set latency BEFORE increasing clock speed.
 *===========================================================================*/

typedef struct {
    volatile uint32_t ACR;      /* Access control             offset 0x00 */
    volatile uint32_t KEYR;     /* Key register               offset 0x04 */
    volatile uint32_t OPTKEYR;  /* Option key register        offset 0x08 */
    volatile uint32_t SR;       /* Status register            offset 0x0C */
    volatile uint32_t CR;       /* Control register           offset 0x10 */
    volatile uint32_t OPTCR;    /* Option control             offset 0x14 */
} FLASH_TypeDef;

#define FLASH   ((FLASH_TypeDef *) FLASH_BASE)

/* FLASH ACR bit positions */
#define FLASH_ACR_LATENCY_0WS   (0U << 0)
#define FLASH_ACR_LATENCY_1WS   (1U << 0)
#define FLASH_ACR_LATENCY_2WS   (2U << 0)
#define FLASH_ACR_PRFTEN        (1U << 8)   /* Prefetch enable            */
#define FLASH_ACR_ICEN          (1U << 9)   /* Instruction cache enable   */
#define FLASH_ACR_DCEN          (1U << 10)  /* Data cache enable          */

/*===========================================================================
 * SECTION 16 — IWDG / WWDG (Watchdogs) (RM0368 Chapter 17-18)
 *===========================================================================*/

typedef struct {
    volatile uint32_t KR;       /* Key register               offset 0x00 */
    volatile uint32_t PR;       /* Prescaler register         offset 0x04 */
    volatile uint32_t RLR;      /* Reload register            offset 0x08 */
    volatile uint32_t SR;       /* Status register            offset 0x0C */
} IWDG_TypeDef;

typedef struct {
    volatile uint32_t CR;       /* Control register           offset 0x00 */
    volatile uint32_t CFR;      /* Configuration register     offset 0x04 */
    volatile uint32_t SR;       /* Status register            offset 0x08 */
} WWDG_TypeDef;

#define IWDG    ((IWDG_TypeDef *) IWDG_BASE)
#define WWDG    ((WWDG_TypeDef *) WWDG_BASE)

/* IWDG key values */
#define IWDG_KR_RELOAD          0xAAAAU     /* Reload watchdog counter    */
#define IWDG_KR_START           0xCCCCU     /* Start watchdog             */
#define IWDG_KR_UNLOCK          0x5555U     /* Unlock PR and RLR          */

/*===========================================================================
 * SECTION 17 — CRC (RM0368 Chapter 4)
 *===========================================================================*/

typedef struct {
    volatile uint32_t DR;       /* Data register              offset 0x00 */
    volatile uint8_t  IDR;      /* Independent data (8-bit)   offset 0x04 */
    uint8_t           RESERVED0;
    uint16_t          RESERVED1;
    volatile uint32_t CR;       /* Control register           offset 0x08 */
} CRC_TypeDef;

#define CRC     ((CRC_TypeDef *) CRC_BASE)

#endif /* STM32F401XX_H */
