#pragma once

// Parse one UART line (without trailing \r\n) from the STM32.
// Frame formats:
//   @IMU:yaw,pitch,roll;;
//   @RPM:rpm;;
// Values are floats, as sent by the STM32 (degrees / motor RPM).

#ifdef __cplusplus
extern "C" {
#endif

// Parse "@IMU:yaw,pitch,roll;;" into out params.
// Return 1 on success, 0 if the line is not a valid IMU frame.
int parse_imu(const char* line, float* yaw, float* pitch, float* roll);

// Parse "@RPM:rpm;;" into out param.
// Return 1 on success, 0 if the line is not a valid RPM frame.
int parse_rpm(const char* line, float* rpm);

#ifdef __cplusplus
}
#endif
