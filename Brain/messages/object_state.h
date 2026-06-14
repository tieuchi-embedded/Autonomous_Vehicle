#pragma once
#include "message.h"

// Single most-relevant detected object (e.g. nearest obstacle / sign).
// Published by object_detection node, consumed by planning.
typedef struct {
    MessageHeader h;
    uint32_t cls;        // class id (0 = none / no detection)
    float    distance;   // meters to the object, forward (+)
    float    confidence; // detection score, 0..1
} ObjectState;
