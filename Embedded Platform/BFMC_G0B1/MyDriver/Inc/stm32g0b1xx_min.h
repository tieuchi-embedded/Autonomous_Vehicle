/**
 ******************************************************************************
 * @file    stm32g0b1xx_min.h
 * @brief   Minimal STM32G0B1xx peripheral register definitions
 *          Based on RM0444 - STM32G0x1 Reference Manual
 *
 * Target: STM32G0B1RE (Cortex-M0+, single AHB/APB bus, 64MHz max)
 *
 * Key differences from STM32F411 (RCC and peripheral programming model):
 *   - Cortex-M0+ : NVIC has only ISER[1]/ICER[1]/ISPR[1]/ICPR[1] (32 IRQ regs ok up to 64 lines)
 *   - RCC: single AHBENR + APBENR1/APBENR2 (no AHB1/APB1/APB2 split), GPIO clocks under IOPENR
 *   - GPIO: same MODER/OTYPER/OSPEEDR/PUPDR/AFR layout as F4 (compatible)
 *   - I2C: v2 peripheral — TIMINGR-based timing, ISR/TXDR/RXDR (no SR1/SR2/DR/CCR/TRISE)
 *   - USART: v2 peripheral — ISR/TDR/RDR (no SR/DR), PRESC register
 *   - HSI = 16 MHz default, no PLL needed for 16 MHz operation
 *===========================================================================*/

#ifndef STM32G0B1XX_MIN_H
#define STM32G0B1XX_MIN_H

#include <stdint.h>

/*===========================================================================
 * SECTION 1 — CORE REGISTERS (Cortex-M0+, ARM-defined)
 *===========================================================================*/

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_TypeDef;

#define SYSTICK_BASE    0xE000E010U
#define SYSTICK         ((SysTick_TypeDef *) SYSTICK_BASE)

/* NVIC — Cortex-M0+ has a reduced NVIC (only ISER[0]/ICER[0]/ISPR[0]/ICPR[0]) */
typedef struct {
    volatile uint32_t ISER[1];
    uint32_t          RESERVED0[31];
    volatile uint32_t ICER[1];
    uint32_t          RESERVED1[31];
    volatile uint32_t ISPR[1];
    uint32_t          RESERVED2[31];
    volatile uint32_t ICPR[1];
    uint32_t          RESERVED3[31];
    uint32_t          RESERVED4[64];
    volatile uint32_t IPR[8];
} NVIC_TypeDef;

#define NVIC_BASE       0xE000E100U
#define NVIC            ((NVIC_TypeDef *) NVIC_BASE)

typedef struct {
    volatile uint32_t CPUID;
    volatile uint32_t ICSR;
    volatile uint32_t VTOR;
    volatile uint32_t AIRCR;
    volatile uint32_t SCR;
    volatile uint32_t CCR;
    uint32_t          RESERVED0;
    volatile uint32_t SHPR2;
    volatile uint32_t SHPR3;
} SCB_TypeDef;

#define SCB_BASE        0xE000ED00U
#define SCB             ((SCB_TypeDef *) SCB_BASE)

/* Cortex-M0+ has no FPU — FPU_Enable() in main.c must be removed/no-op for G0B1 */

/*===========================================================================
 * SECTION 2 — BUS / PERIPHERAL BASE ADDRESSES (RM0444 memory map)
 *===========================================================================*/

#define PERIPH_BASE         0x40000000U

#define APBPERIPH_BASE      (PERIPH_BASE + 0x00000000U)
#define AHBPERIPH_BASE      (PERIPH_BASE + 0x00020000U)
#define IOPPERIPH_BASE      0x50000000U

/* APB peripherals */
#define TIM2_BASE           (APBPERIPH_BASE + 0x0000U)
#define TIM3_BASE           (APBPERIPH_BASE + 0x0400U)
#define IWDG_BASE           (APBPERIPH_BASE + 0x3000U)
#define USART2_BASE         (APBPERIPH_BASE + 0x4400U)
#define I2C1_BASE           (APBPERIPH_BASE + 0x5400U)
#define PWR_BASE            (APBPERIPH_BASE + 0x7000U)
#define TIM1_BASE           (APBPERIPH_BASE + 0x12C00U)
#define USART1_BASE         (APBPERIPH_BASE + 0x13800U)
#define SYSCFG_BASE         (APBPERIPH_BASE + 0x10000U)

/* AHB peripherals */
#define RCC_BASE            (AHBPERIPH_BASE + 0x1000U)
#define FLASH_BASE          (AHBPERIPH_BASE + 0x2000U)

/* IOPORT (GPIO) peripherals */
#define GPIOA_BASE          (IOPPERIPH_BASE + 0x0000U)
#define GPIOB_BASE          (IOPPERIPH_BASE + 0x0400U)
#define GPIOC_BASE          (IOPPERIPH_BASE + 0x0800U)
#define GPIOD_BASE          (IOPPERIPH_BASE + 0x0C00U)
#define GPIOE_BASE          (IOPPERIPH_BASE + 0x1000U)
#define GPIOF_BASE          (IOPPERIPH_BASE + 0x1400U)

/*===========================================================================
 * SECTION 3 — GPIO (RM0444 Ch.17) — same layout as F4
 *===========================================================================*/

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
    volatile uint32_t BRR;
} GPIO_TypeDef;

#define GPIOA   ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB   ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC   ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD   ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE   ((GPIO_TypeDef *) GPIOE_BASE)
#define GPIOF   ((GPIO_TypeDef *) GPIOF_BASE)

/*===========================================================================
 * SECTION 4 — RCC (RM0444 Ch.5)
 *
 * Single AHB/APB bus design — no AHB1/AHB2/APB1/APB2 split like F4:
 *   GPIO clocks   -> RCC->IOPENR
 *   DMA/CRC/Flash -> RCC->AHBENR
 *   TIM2/3,I2C1,USART2,PWR -> RCC->APBENR1
 *   SYSCFG,TIM1,USART1     -> RCC->APBENR2
 *
 * Default clock: HSI 16 MHz -> SYSCLK -> AHB -> APB all at 16 MHz (no PLL)
 *===========================================================================*/

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t ICSCR;
    volatile uint32_t CFGR;
    volatile uint32_t PLLCFGR;
    uint32_t          RESERVED0;
    volatile uint32_t CRRCR;
    volatile uint32_t CIER;
    volatile uint32_t CIFR;
    volatile uint32_t CICR;
    volatile uint32_t IOPRSTR;
    volatile uint32_t AHBRSTR;
    volatile uint32_t APBRSTR1;
    volatile uint32_t APBRSTR2;
    volatile uint32_t IOPENR;
    volatile uint32_t AHBENR;
    volatile uint32_t APBENR1;
    volatile uint32_t APBENR2;
    volatile uint32_t IOPSMENR;
    volatile uint32_t AHBSMENR;
    volatile uint32_t APBSMENR1;
    volatile uint32_t APBSMENR2;
    volatile uint32_t CCIPR;
    volatile uint32_t CCIPR2;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_TypeDef;

#define RCC     ((RCC_TypeDef *) RCC_BASE)

/* RCC IOPENR — GPIO port clock enables */
#define RCC_IOPENR_GPIOAEN      (1U << 0)
#define RCC_IOPENR_GPIOBEN      (1U << 1)
#define RCC_IOPENR_GPIOCEN      (1U << 2)
#define RCC_IOPENR_GPIODEN      (1U << 3)
#define RCC_IOPENR_GPIOEEN      (1U << 4)
#define RCC_IOPENR_GPIOFEN      (1U << 5)

/* RCC APBENR1 */
#define RCC_APBENR1_TIM2EN      (1U << 0)
#define RCC_APBENR1_TIM3EN      (1U << 1)
#define RCC_APBENR1_USART2EN    (1U << 17)
#define RCC_APBENR1_I2C1EN      (1U << 21)
#define RCC_APBENR1_PWREN       (1U << 28)

/* RCC APBENR2 */
#define RCC_APBENR2_SYSCFGEN    (1U << 0)
#define RCC_APBENR2_TIM1EN      (1U << 11)
#define RCC_APBENR2_USART1EN    (1U << 14)

/* RCC CR */
#define RCC_CR_HSION            (1U << 8)
#define RCC_CR_HSIRDY           (1U << 10)

/*===========================================================================
 * SECTION 5 — USART (RM0444 Ch.27) — v2 peripheral, ISR/TDR/RDR (NOT SR/DR)
 *
 * G0B1 has: USART1, USART2, USART3, USART4, LPUART1, LPUART2
 * BRR = fCLK / baud  (OVER8=0, default — same formula as F4 USARTDIV)
 *===========================================================================*/

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t BRR;
    volatile uint32_t GTPR;
    volatile uint32_t RTOR;
    volatile uint32_t RQR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t RDR;
    volatile uint32_t TDR;
    volatile uint32_t PRESC;
} USART_TypeDef;

#define USART1  ((USART_TypeDef *) USART1_BASE)
#define USART2  ((USART_TypeDef *) USART2_BASE)

/* USART ISR bit positions (replaces F4's SR) */
#define USART_ISR_PE            (1U << 0)
#define USART_ISR_FE            (1U << 1)
#define USART_ISR_NE            (1U << 2)
#define USART_ISR_ORE           (1U << 3)
#define USART_ISR_IDLE          (1U << 4)
#define USART_ISR_RXNE_RXFNE    (1U << 5)   /* RXNE on G0 (RXFNE in newer naming) */
#define USART_ISR_TC            (1U << 6)
#define USART_ISR_TXE_TXFNF     (1U << 7)   /* TXE on G0 (TXFNF in newer naming)  */

/* USART CR1 bit positions — same positions as F4 */
#define USART_CR1_RE            (1U << 2)
#define USART_CR1_TE            (1U << 3)
#define USART_CR1_RXNEIE        (1U << 5)
#define USART_CR1_PCE           (1U << 10)
#define USART_CR1_M0            (1U << 12)
#define USART_CR1_UE            (1U << 0)

/*===========================================================================
 * SECTION 6 — I2C (RM0444 Ch.32) — v2 peripheral, TIMINGR + ISR/TXDR/RXDR
 *
 * G0B1 has: I2C1, I2C2, I2C3
 * Programming model is fundamentally different from F4 I2C (which used
 * SR1/SR2/DR/CCR/TRISE byte-by-byte polling). G0 I2C uses CR2 to set
 * SADD/NBYTES/RD_WRN/START/AUTOEND/STOP in one shot, then poll TXIS/RXNE/TC.
 *===========================================================================*/

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t TIMINGR;
    volatile uint32_t TIMEOUTR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t PECR;
    volatile uint32_t RXDR;
    volatile uint32_t TXDR;
} I2C_TypeDef;

#define I2C1    ((I2C_TypeDef *) I2C1_BASE)

/* I2C CR1 */
#define I2C_CR1_PE              (1U << 0)

/* I2C CR2 */
#define I2C_CR2_SADD_Pos        (0U)
#define I2C_CR2_RD_WRN          (1U << 10)
#define I2C_CR2_START           (1U << 13)
#define I2C_CR2_STOP            (1U << 14)
#define I2C_CR2_NACK            (1U << 15)
#define I2C_CR2_NBYTES_Pos      (16U)
#define I2C_CR2_RELOAD          (1U << 24)
#define I2C_CR2_AUTOEND         (1U << 25)

/* I2C ISR */
#define I2C_ISR_TXE             (1U << 0)
#define I2C_ISR_TXIS            (1U << 1)
#define I2C_ISR_RXNE            (1U << 2)
#define I2C_ISR_NACKF           (1U << 4)
#define I2C_ISR_STOPF           (1U << 5)
#define I2C_ISR_TC              (1U << 6)
#define I2C_ISR_TCR             (1U << 7)
#define I2C_ISR_BUSY            (1U << 15)

/* I2C ICR */
#define I2C_ICR_NACKCF          (1U << 4)
#define I2C_ICR_STOPCF          (1U << 5)

/*===========================================================================
 * SECTION 7 — TIMERS (RM0444 Ch.20-22) — same CR1/CCMR/CCER/PSC/ARR layout as F4
 *
 * G0B1 timers used here: TIM2 (32-bit, APB), TIM3 (16-bit, APB)
 *===========================================================================*/

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RCR;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t BDTR;
    volatile uint32_t DCR;
    volatile uint32_t DMAR;
} TIM_TypeDef;

#define TIM1    ((TIM_TypeDef *) TIM1_BASE)
#define TIM2    ((TIM_TypeDef *) TIM2_BASE)
#define TIM3    ((TIM_TypeDef *) TIM3_BASE)

#define TIM_CR1_CEN             (1U << 0)
#define TIM_CR1_ARPE            (1U << 7)

#define TIM_SR_UIF              (1U << 0)

/*===========================================================================
 * SECTION 8 — IWDG (RM0444 Ch.34) — same layout as F4
 *===========================================================================*/

typedef struct {
    volatile uint32_t KR;
    volatile uint32_t PR;
    volatile uint32_t RLR;
    volatile uint32_t SR;
    volatile uint32_t WINR;
} IWDG_TypeDef;

#define IWDG    ((IWDG_TypeDef *) IWDG_BASE)

#define IWDG_KR_RELOAD          0xAAAAU
#define IWDG_KR_START           0xCCCCU
#define IWDG_KR_UNLOCK          0x5555U

/*===========================================================================
 * SECTION 9 — IRQ numbers used (RM0444 / device header)
 *===========================================================================*/

#define USART2_IRQn             28
#define USART1_IRQn             27
#define TIM2_IRQn               15
#define TIM3_IRQn               16   /* shared TIM3_TIM4_IRQn */

#endif /* STM32G0B1XX_MIN_H */
