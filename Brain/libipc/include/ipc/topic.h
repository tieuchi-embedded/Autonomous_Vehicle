#pragma once
#include <stddef.h>
#include <stdint.h>

typedef enum {
    INVALID      = 0,

    // Phase 1 — Perception
    CAMERA_FRAME = 1,  // SHM  NEW
    IMU_STATE    = 2,  // MQ   OLD
    WHEEL_ODOM   = 3,  // MQ   OLD
    EGO_STATE    = 4,  // MQ   OLD
    LANE_STATE   = 5,  // MQ   NEW (depth=1)

    // Phase 3 — Control
    CONTROL_CMD  = 6,  // MQ   NEVER

    TOPIC_COUNT
} TopicId;

typedef enum {
    SHM = 1,
    MQ  = 2
} TransportKind;

typedef enum {
    NEW   = 1,  // overwrite oldest — for SHM and depth=1 MQ
    OLD   = 2,  // drain oldest, retry send
    NEVER = 3   // return EAGAIN if full
} DropPolicy;

typedef struct {
    const char*   name;          // POSIX name: /<topic>
    TransportKind transport;
    size_t        payload_size;  // 0 for CAMERA_FRAME (variable, set by publisher)
    uint16_t      depth;         // SHM: n_slots, MQ: mq_maxmsg
    DropPolicy    drop;
} TopicDescriptor;

const TopicDescriptor* topic_get(TopicId id);  // returns NULL if id invalid
