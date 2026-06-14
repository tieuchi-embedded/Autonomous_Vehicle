/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body — STM32G0B1RET6 (port from F411CEU6)
 ******************************************************************************
 * Cortex-M0+ has no FPU — FPU_Enable() removed (was needed on F411's M4F).
 * HSI16 = 16 MHz default, no PLL — clock assumptions match F411 build (16MHz).
 ******************************************************************************
 */

#include <stdint.h>
#include <stdlib.h>
#include "main.h"

uint8_t imu_available = 0;
extern volatile uint32_t ms_tick;
float deg = 0;
uint8_t id = 0;


void I2C_Scanner(void)
{
    UART2_print_log("\r\n--- BAT DAU QUET I2C ---\r\n");
    uint8_t count = 0;

    for (uint8_t addr = 1; addr < 128; addr++) {
        // Xóa các cờ lỗi cũ trước khi quét
        I2C1->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;

        // Cấu hình thanh ghi CR2: Gửi 0 byte, tự động sinh STOP, tạo START
        I2C1->CR2 = ((uint32_t)addr << 1)
                  | (0U << I2C_CR2_NBYTES_Pos)
                  | I2C_CR2_AUTOEND
                  | I2C_CR2_START;

        // Chờ phản hồi từ đường truyền (chờ STOPF hoặc NACKF)
        // Mình thêm biến timeout để tránh treo vòng lặp nếu bus bị kẹt cứng
        uint32_t timeout = 10000;
        while (!(I2C1->ISR & (I2C_ISR_STOPF | I2C_ISR_NACKF))) {
            if (--timeout == 0) break;
        }

        if (timeout == 0) {
            UART2_print_log("Loi: Bus I2C bi treo hoan toan!\r\n");
            break; // Thoát vòng lặp luôn vì bus đã kẹt
        }

        if (I2C1->ISR & I2C_ISR_NACKF) {
            // Nhận NACK -> Không có ai ở địa chỉ này
            I2C1->ICR = I2C_ICR_NACKCF;
        } else if (I2C1->ISR & I2C_ISR_STOPF) {
            // Nhận ACK -> Có thiết bị phản hồi!
            UART2_print_log("=> TIM THAY thiet bi o dia chi (He Dec): ");
            UART2_send_number(addr);
            UART2_print_log("\r\n");
            count++;
        }

        I2C1->ICR = I2C_ICR_STOPCF; // Xóa cờ STOP
        Delay_t(2); // Nghỉ 2ms trước khi gọi địa chỉ tiếp theo
    }

    if (count == 0) {
        UART2_print_log("Khong tim thay bat ky thiet bi I2C nao tren duong truyen.\r\n");
    }
    UART2_print_log("--- KET THUC QUET ---\r\n");
}

int main(void)
{
    RCC_Init();

    UART2_init();
    I2C1_Master_Init();
    PWM_Init();
    Timer3_Init();
    SysTick_Init(CPU_FREQ_HZ);
    Delay_t(1000);

    I2C_Scanner();

    UART2_print_log("\r\n");
    UART2_print_log("BNO055 scan...\r\n");
    id = BNO055_ReadReg(BNO055_CHIP_ID);
    UART2_print_log("CHIP_ID=");
    UART2_send_number(id);
    UART2_print_log("\r\n");
    if (id != BNO055_CHIP_ID_VAL) {
        UART2_print_log("BNO055 not found, degraded mode\r\n");
        imu_available = 0;
    } else {
        imu_available = 1;
    }
    UART2_print_log("BNO055 __init__...\r\n");
    BNO055_Init();
//
//    uint8_t as_stat = AS5600_ReadStatus();
//    UART2_print_log("AS5600 STATUS=");
//    UART2_send_number(as_stat);
//    UART2_print_log("\r\n");
//
//    Watchdog_Init();
    Servo_Speed_Set(1500);
    Servo_Steering_Set_Deg(0);
    Delay_t(2000);


    while (1)
    {
//        Servo_Speed_Set(1800);
//        Delay_t(1000);
//        Servo_Speed_Set(1500);
//        Delay_t(1000);
//        Servo_Speed_Set(1200);
//        Delay_t(1000);
//        Servo_Speed_Set(1500);
//        Delay_t(1000);

        if (imu_available) {
            int16_t ex, ey, ez;
            float yaw, roll, pitch;

            BNO055_ReadEuler_raw(&ex, &ey, &ez);

            yaw   = ex / 16.0f;
            roll  = ey / 16.0f;
            pitch = ez / 16.0f;

            UART2_print_log("@IMU:");
            UART2_send_float(yaw);
            UART2_print_log(",");
            UART2_send_float(pitch);
            UART2_print_log(",");
            UART2_send_float(roll);
            UART2_print_log(";;");
            UART2_print_log("\r\n");

            IMU_HealthCheck(ex, ey, ez);
        }

        deg = AS5600_ReadAngleDeg();
        UART2_print_log("@RPM:");
        UART2_send_float(deg);
        UART2_print_log(";;");
        UART2_print_log("\r\n");
        Delay_t(100);
//        Feed_WD();



    }
}
