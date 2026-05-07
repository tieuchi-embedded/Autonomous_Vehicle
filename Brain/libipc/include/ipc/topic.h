#pragma once
#include <stddef.h>
#include <stdint.h>

typedef enum {
    INVALID      = 0,

    CAMERA_FRAME = 1,  // SHM  NEW
    SERIAL       = 2,  // MQ   OLD  — raw sensor data from serial port(s)
    EGO_STATE    = 3,  // MQ   OLD
    LANE_STATE   = 4,  // MQ   NEW
    CONTROL_CMD  = 5,  // MQ   NEVER
    DETECTIONS   = 6,  // MQ   NEW
    BEHAVIOR_CMD = 7,  // MQ   NEVER
    POSE2D       = 8,  // MQ   OLD

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
