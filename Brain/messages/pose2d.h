#pragma once
#include "message.h"

typedef struct {
    MessageHeader h;
    float x;        // meters
    float y;        // meters
    float heading;  // radians
} Pose2D;
