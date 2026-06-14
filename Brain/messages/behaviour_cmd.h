#pragma once
#include "message.h"

typedef struct {
    MessageHeader h;
    float    target_speed;    // m/s, 0 = stop
    float    target_heading;  // degrees, relative to current yaw
    uint32_t mode;            // 0=idle, 1=lane_follow, 2=stop, 3=obstacle_avoid
} BehaviourCmd;
