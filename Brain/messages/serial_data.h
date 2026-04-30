#pragma once
#include "message.h"

// Raw sensor data aggregated from all serial ports.
// Published by serial_node(s), consumed by state_node.
typedef struct {
    MessageHeader h;
    float yaw;    // degrees
    float pitch;  // degrees
    float roll;   // degrees
    float wx;     // rad/s — reserved, zero for now
    float wy;     // rad/s — reserved, zero for now
    float wz;     // rad/s — reserved, zero for now
    float ax;     // m/s²  — reserved, zero for now
    float ay;     // m/s²  — reserved, zero for now
    float az;     // m/s²  — reserved, zero for now
    float rpm;    // motor RPM
} SerialData;
