/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body — STM32F411CEU6
 ******************************************************************************
 */

#include <stdint.h>
#include "main.h"

static inline void FPU_Enable(void)
{
    SCB_CPACR |= (0xFU << 20);
    __asm volatile("DSB");
    __asm volatile("ISB");
}

uint8_t imu_available = 0;
extern volatile uint32_t ms_tick;

int main(void)
{
    FPU_Enable();

    UART2_init();
    I2C1_Master_Init();
    PWM_Init();
    Timer3_Init();
    SysTick_Init(16000000);

//    UART2_print_log("\r\n");
//    UART2_print_log("BNO055 scan...\r\n");
//    uint8_t id = BNO055_ReadReg(BNO055_CHIP_ID);
//    UART2_print_log("CHIP_ID=");
//    UART2_send_number(id);
//    UART2_print_log("\r\n");
//    if (id != BNO055_CHIP_ID_VAL) {
//        UART2_print_log("BNO055 not found, degraded mode\r\n");
//        imu_available = 0;
//    } else {
//        imu_available = 1;
//    }
//    UART2_print_log("BNO055 __init__...\r\n");
//    BNO055_Init();
//
//    uint8_t as_stat = AS5600_ReadStatus();
//    UART2_print_log("AS5600 STATUS=");
//    UART2_send_number(as_stat);
//    UART2_print_log("\r\n");

    Watchdog_Init();
    Servo_Speed_Set(1500);
    Delay_t(2000);


    while (1)
    {
        Servo_Speed_Set(1700);
        Delay_t(1000);
        Servo_Speed_Set(1500);
        Delay_t(1000);
        Servo_Speed_Set(1300);
        Delay_t(1000);
        Servo_Speed_Set(1500);
        Delay_t(1000);

//        if (imu_available) {
//            int16_t ex, ey, ez;
//            float yaw, roll, pitch;
//
//            BNO055_ReadEuler_raw(&ex, &ey, &ez);
//
//            yaw   = ex / 16.0f;
//            roll  = ey / 16.0f;
//            pitch = ez / 16.0f;
//
//            UART2_print_log("@IMU:");
//            UART2_send_float(yaw);
//            UART2_print_log(",");
//            UART2_send_float(pitch);
//            UART2_print_log(",");
//            UART2_send_float(roll);
//            UART2_print_log(";;");
//            UART2_print_log("\r\n");
//
//            IMU_HealthCheck(ex, ey, ez);
//        }
//
//        float deg = AS5600_ReadAngleDeg();
//        UART2_print_log("@RPM:");
//        UART2_send_float(deg);
//        UART2_print_log(";;");
//        UART2_print_log("\r\n");

        Feed_WD();



    }
}
