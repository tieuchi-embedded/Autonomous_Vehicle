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


static float sim_yaw = 0.0f;
static float sim_roll = 0.0f;
static float sim_pitch = 0.0f;


void Simulate_BNO055_Euler(float *yaw, float *roll, float *pitch) {
    // 1. Tạo độ lệch ngẫu nhiên (delta) cho mỗi trục.
    // Giả sử mỗi chu kỳ chỉ xoay tối đa khoảng +/- 2.5 độ
    float delta_yaw   = ((float)rand() / RAND_MAX) * 5.0f - 2.5f;
    float delta_roll  = ((float)rand() / RAND_MAX) * 5.0f - 2.5f;
    float delta_pitch = ((float)rand() / RAND_MAX) * 5.0f - 2.5f;

    // 2. Cập nhật góc hiện tại
    sim_yaw   += delta_yaw;
    sim_roll  += delta_roll;
    sim_pitch += delta_pitch;

    // 3. Xử lý giới hạn (Wrap) cho Yaw (0 đến 360 độ)
    if (sim_yaw >= 360.0f) {
        sim_yaw -= 360.0f;
    } else if (sim_yaw < 0.0f) {
        sim_yaw += 360.0f;
    }

    // 4. Xử lý giới hạn (Clamp hoặc Bounce) cho Roll (-90 đến 90 độ)
    if (sim_roll > 90.0f) {
        sim_roll = 90.0f;
        // Nếu muốn hiệu ứng "nảy" lại khi chạm giới hạn, bạn có thể đảo dấu delta ở chu kỳ sau,
        // nhưng với mock data đơn giản thì clamp là đủ.
    } else if (sim_roll < -90.0f) {
        sim_roll = -90.0f;
    }

    // 5. Xử lý giới hạn (Clamp) cho Pitch (-180 đến 180 độ)
    if (sim_pitch > 180.0f) {
        sim_pitch = 180.0f;
    } else if (sim_pitch < -180.0f) {
        sim_pitch = -180.0f;
    }

    // 6. Gán giá trị đầu ra
    *yaw   = sim_yaw;
    *roll  = sim_roll;
    *pitch = sim_pitch;
}

int main(void)
{
    RCC_Init();

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

//        if (imu_available) {
////            int16_t ex, ey, ez;
//            float yaw, roll, pitch;
////
////            BNO055_ReadEuler_raw(&ex, &ey, &ez);
////
////            yaw   = ex / 16.0f;
////            roll  = ey / 16.0f;
////            pitch = ez / 16.0f;
////
//    	Simulate_BNO055_Euler(&yaw, &roll, &pitch);
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

//        deg = AS5600_ReadAngleDeg();
//        UART2_print_log("@RPM:");
//        UART2_send_float(deg);
//        UART2_print_log(";;");
//        UART2_print_log("\r\n");
//        Delay_t(100);
//        Feed_WD();



    }
}
