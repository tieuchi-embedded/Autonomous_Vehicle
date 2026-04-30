#pragma once
#include "message.h"

typedef struct {
    MessageHeader h;
    uint32_t time_ms;  // ms since state_node start
    float    angle;    // degrees — yaw from IMU
    float    speed;    // cm/s   — computed from RPM
} EgoState;
