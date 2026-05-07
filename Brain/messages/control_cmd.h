#pragma once
#include "message.h"

typedef struct {
    MessageHeader h;
    float rpm;        // motor RPM, -500..+500, +forward / -reverse / 0=stop
    float steer_deg;  // steering degrees, -30..+30, +right / -left / 0=straight
} ControlCmd;
