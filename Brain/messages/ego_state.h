#pragma once
#include "message.h"

typedef struct {
    MessageHeader h;
    float yaw;    // radians
    float pitch;  // radians
    float roll;   // radians
} EgoState;
