/* SPDX-License-Identifier: MIT */
/*
 * topic.h - Topic registry: identifiers and metadata for libipc channels.
 *
 * Every data stream exchanged over the bus is a *topic*. A topic is named
 * by a TopicId enum value, which is also the index into a compile-time
 * table of TopicDescriptor entries (see topic.c). The descriptor is the
 * single source of truth that maps an id to everything the transport layer
 * needs: kernel object name, transport kind, message size, queue depth and
 * drop policy.
 *
 * Adding a topic means: add an enum value here (before TOPIC_COUNT) and a
 * matching row in the descriptor table in topic.c. Enum values are stable
 * wire identifiers and are stored in every MessageHeader, so do not
 * renumber existing topics.
 */
#pragma once
#include <stddef.h>
#include <stdint.h>

/**
 * enum TopicId - Stable identifier for each bus topic.
 * @INVALID:       reserved zero value; never a valid topic.
 * @CAMERA_FRAME:  raw camera frames (variable size).
 * @EGO_STATE:     raw vehicle telemetry from the serial port (yaw/pitch/roll, rpm).
 * @LANE_STATE:    lane detection output (heading error, lateral offset).
 * @CONTROL_CMD:   low-level motor/servo commands.
 * @OBJECT_STATE:  object detection results (variable size).
 * @BEHAVIOUR_CMD: high-level behaviour commands (target speed/heading).
 * @LOCATION:      2D pose estimate (x, y, heading).
 * @TOPIC_COUNT:   number of topics; also the size of the descriptor table.
 *
 * Used directly as the index into the TopicDescriptor table in topic.c.
 */
typedef enum {
    INVALID       = 0,

    CAMERA_FRAME  = 1,
    EGO_STATE     = 2,
    LANE_STATE    = 3,
    CONTROL_CMD   = 4,
    OBJECT_STATE  = 5,
    BEHAVIOUR_CMD = 6,
    LOCATION      = 7,

    TOPIC_COUNT
} TopicId;

/**
 * enum TransportKind - Underlying IPC mechanism backing a topic.
 * @SHM: shared memory, latest-wins. Best for high-rate state where only the
 *       newest sample matters; readers may miss intermediate values.
 * @MQ:  POSIX message queue, ordered FIFO with bounded depth. Best for
 *       commands/events that must be delivered in order.
 */
typedef enum {
    SHM = 1,
    MQ  = 2
} TransportKind;

/**
 * struct TopicDescriptor - Static metadata describing one topic.
 * @name:         POSIX object name, of the form "/<topic>".
 * @transport:    SHM or MQ (see enum TransportKind).
 * @payload_size: fixed message size in bytes, or 0 for variable-size topics
 *                (e.g. CAMERA_FRAME) where the publisher sets the size.
 * @depth:        SHM: number of slots; MQ: mq_maxmsg (clamped to the system
 *                limit /proc/sys/fs/mqueue/msg_max at open time).
 *
 * Both transports are latest-wins: when an MQ queue is full the oldest
 * message is dropped to make room for the newest one.
 */
typedef struct {
    const char*   name;
    TransportKind transport;
    size_t        payload_size;
    uint16_t      depth;
} TopicDescriptor;

/**
 * topic_get() - Look up the descriptor for a topic id.
 * @id: topic identifier.
 *
 * Return: pointer to the (immutable, statically allocated) descriptor, or
 * NULL if @id is out of range. The returned pointer is valid for the
 * lifetime of the process.
 */
const TopicDescriptor* topic_get(TopicId id);
