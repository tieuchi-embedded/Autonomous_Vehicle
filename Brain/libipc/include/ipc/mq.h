/* SPDX-License-Identifier: MIT */
/*
 * mq.h - POSIX message queue transport for libipc (internal).
 *
 * Thin wrapper over the POSIX mqueue API (mq_open/mq_send/mq_receive) that
 * implements the MQ side of the bus. An MQ topic is an ordered FIFO with a
 * bounded depth; it is the right transport for commands and events that must
 * be delivered in order and not silently dropped.
 *
 * This is an internal interface used by ipc.c. Applications should use the
 * public API in ipc/ipc.h and never call these functions directly.
 *
 * The queue depth requested in the topic descriptor is clamped at open time
 * to the system limit in /proc/sys/fs/mqueue/msg_max.
 */
#pragma once
#include "ipc/topic.h"
#include <mqueue.h>

/**
 * struct MqHandle - Open message-queue endpoint.
 * @mqd: POSIX message queue descriptor, or (mqd_t)-1 when closed.
 */
typedef struct { mqd_t mqd; } MqHandle;

/**
 * mq_publish() - Open (creating if needed) a topic queue for writing.
 * @id:  topic identifier.
 * @out: handle to initialize on success.
 *
 * Return: 0 on success, -1 on error (invalid id or mq_open failure).
 */
int  mq_publish(TopicId id, MqHandle* out);

/**
 * mq_subscribe() - Open (creating if needed) a topic queue for reading.
 * @id:  topic identifier.
 * @out: handle to initialize on success.
 *
 * Return: 0 on success, -1 on error (invalid id or mq_open failure).
 */
int  mq_subscribe(TopicId id, MqHandle* out);

/**
 * mq_push() - Enqueue one message (latest-wins).
 * @h:    open queue handle.
 * @td:   descriptor of the topic being sent on (provides the message size).
 * @msg:  message payload.
 * @size: byte size of @msg.
 *
 * If the queue is full, the oldest message(s) are dropped to make room and
 * @msg is enqueued at the tail. Never blocks.
 *
 * Return: 0 on success, -1 on error.
 */
int  mq_push(MqHandle* h, const TopicDescriptor* td, const void* msg, size_t size);

/**
 * mq_pull() - Dequeue the oldest message, waiting up to a timeout.
 * @h:          open queue handle.
 * @buf:        destination buffer.
 * @size:       capacity of @buf in bytes.
 * @timeout_ms: maximum time to wait for a message, in milliseconds.
 *
 * Return: 0 if a message was received, 1 on timeout / no data, -1 on error.
 */
int  mq_pull(MqHandle* h, void* buf, size_t size, int timeout_ms);

/**
 * mq_disconnect() - Close a queue handle.
 * @h: handle to close; safe to call more than once.
 *
 * Closes the descriptor but does not remove the queue from the system
 * (see mq_remove()).
 */
void mq_disconnect(MqHandle* h);

/**
 * mq_remove() - Remove a topic's queue from the system.
 * @id: topic identifier.
 *
 * Unlinks the named queue so it is destroyed once all descriptors are
 * closed. Typically called by the publisher on shutdown.
 */
void mq_remove(TopicId id);
