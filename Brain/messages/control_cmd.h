#pragma once
#include "message.h"

// TODO Phase 3 — steering unit not finalized (radians or normalized [-1,1]?)
typedef struct {
    MessageHeader h;
    float speed;    // m/s
    float steering;
    float brake;    // [0,1]
} ControlCmd;
