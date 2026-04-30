#pragma once
#include "message.h"

typedef struct {
    MessageHeader h;
    float roll, pitch, yaw;  // radians
    float wx, wy, wz;        // rad/s
    float ax, ay, az;        // m/s²
} ImuState;
