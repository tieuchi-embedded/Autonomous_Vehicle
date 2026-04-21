#pragma once
#include <stdint.h>

typedef struct {
    uint32_t topic;  // TopicId
    uint32_t seq;    // monotonic counter, incremented on each publish
    uint64_t ts_ns;  // CLOCK_MONOTONIC timestamp, nanoseconds
} MessageHeader;
