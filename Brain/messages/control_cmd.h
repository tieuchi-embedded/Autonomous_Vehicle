#pragma once
#include "message.h"

typedef struct {
    MessageHeader h;
    float rpm;        // motor RPM, +forward / -reverse / 0=stop
    float steer_deg;  // steering angle degrees, + right / - left
} ControlCmd;
