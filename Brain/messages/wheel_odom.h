#pragma once
#include "message.h"

typedef struct {
    MessageHeader h;
    float   speed;  // m/s
    int32_t ticks;  // encoder ticks (cumulative)
    float   dist;   // meters (cumulative)
} WheelOdom;
