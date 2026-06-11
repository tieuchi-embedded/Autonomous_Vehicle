/*
 * mydriver.h — G0B1 port
 */
#include <string.h>
#include <stdlib.h>

#ifndef INC_MYDRIVER_H_
#define INC_MYDRIVER_H_

#include "stm32g0b1xx_min.h"

#define UART2_RX_BUF_SIZE 256
extern volatile uint8_t uart2_rx_dma_buf[UART2_RX_BUF_SIZE];

// ============================================================
// Chỉ sửa dòng này: 16, 32, hoặc 64
// ============================================================
#define SYSCLK_MHZ      16
#define CPU_FREQ_HZ     (SYSCLK_MHZ * 1000000U)

// Auto-calculated — do not edit
#if SYSCLK_MHZ == 16
  #define USE_PLL         0
  #define FLASH_LATENCY   0U

#elif SYSCLK_MHZ == 32
  #define USE_PLL         1
  #define PLL_M_REG       0
  #define PLL_N           4
  #define PLL_R_REG       0
  #define FLASH_LATENCY   1U

#elif SYSCLK_MHZ == 64
  #define USE_PLL         1
  #define PLL_M_REG       0
  #define PLL_N           8
  #define PLL_R_REG       0
  #define FLASH_LATENCY   2U

#else
  #error "SYSCLK_MHZ must be 16, 32, or 64"
#endif

#include "rcc.h"
#include "i2c.h"
#include "uart.h"
#include "bno055.h"
#include "as5600.h"
#include "timer.h"
#include "servo.h"
#include "watchdog.h"
#include "systick.h"

#endif /* INC_MYDRIVER_H_ */
