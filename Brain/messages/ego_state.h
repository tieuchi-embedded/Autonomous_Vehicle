#pragma once
#include "message.h"

typedef struct {
    MessageHeader h;
    float yaw;    // degrees
    float pitch;  // degrees
    float roll;   // degrees
    float rpm;    // motor RPM
} EgoState;
