#pragma once
#include "message.h"

typedef struct {
    MessageHeader h;
    float heading_err_rad;  // angle deviation from lane tangent
} LaneState;
