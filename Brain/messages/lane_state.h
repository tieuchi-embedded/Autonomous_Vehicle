#pragma once
#include "message.h"

typedef struct {
    MessageHeader h;
    float heading_err_rad;   // angle deviation from lane tangent
    float lateral_offset_m;  // lateral offset from lane center (meters, +right)
} LaneState;
