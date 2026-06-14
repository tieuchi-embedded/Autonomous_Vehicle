/* SPDX-License-Identifier: MIT */
/*
 * ipc.h - Unified publish/subscribe API for libipc.
 *
 * This is the single public entry point applications use to exchange
 * messages between independent processes ("nodes"). Each logical data
 * stream is a *topic*, identified by a TopicId (see ipc/topic.h). The
 * topic descriptor decides the underlying transport, so callers never
 * touch shared memory or POSIX message queues directly:
 *
 *   - Shared memory (SHM): latest-wins, single slot. Used for high-rate
 *     state where only the freshest sample matters (camera frames, ego
 *     state). A late reader simply misses intermediate values.
 *   - POSIX message queue (MQ): ordered FIFO with a bounded depth. Used
 *     for commands/events that must not be silently dropped.
 *
 * Ordering constraint: ipc_publish() creates the backing kernel object, so
 * the publisher of a topic MUST run before any subscriber to it.
 * ipc_subscribe() only attaches to (never creates) the object.
 *
 * Lifecycle:
 *   publisher:  ipc_publish() -> ipc_push() ... -> ipc_unpublish()
 *   subscriber: ipc_subscribe() -> ipc_pull() ... -> ipc_unsubscribe()
 *
 * Handles are opaque and not thread-safe: do not share a single
 * publisher or subscriber handle across threads without external
 * locking.
 */
#pragma once
#include "ipc/topic.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque publisher handle. One per (process, topic) publish endpoint. */
typedef struct ipc_publisher  ipc_publisher_t;

/* Opaque subscriber handle. One per (process, topic) receive endpoint. */
typedef struct ipc_subscriber ipc_subscriber_t;

/**
 * ipc_publish() - Open a publish endpoint, creating the topic's backing object.
 * @id:           topic identifier (see TopicId in ipc/topic.h).
 * @payload_size: byte size of the messages to be published. Only meaningful
 *                for SHM topics, where it sizes the shared slot; ignored for
 *                MQ topics, which take their message size from the descriptor.
 *
 * Creates the underlying SHM segment or message queue if it does not yet
 * exist, then returns a handle to push on. Must be called before the matching
 * subscriber.
 *
 * Return: a publisher handle on success, or NULL on failure (invalid topic
 * id, allocation failure, or transport open error).
 */
ipc_publisher_t*  ipc_publish(TopicId id, size_t payload_size);

/**
 * ipc_push() - Push one message to a publish endpoint.
 * @pub:  publisher handle from ipc_publish().
 * @msg:  pointer to the message payload to send.
 * @size: byte size of @msg.
 *
 * Both transports are latest-wins: for SHM the single slot is overwritten;
 * for MQ, if the queue is full the oldest message is dropped to make room.
 * Never blocks.
 *
 * Return: 0 on success, -1 on error.
 */
int               ipc_push(ipc_publisher_t* pub, const void* msg, size_t size);

/**
 * ipc_unpublish() - Close a publish endpoint and release its backing object.
 * @pub: publisher handle, or NULL (no-op).
 *
 * Detaches from the transport and unlinks the kernel object (SHM segment or
 * message queue), then frees the handle. After this call subscribers can
 * no longer receive new data on the topic.
 */
void              ipc_unpublish(ipc_publisher_t* pub);

/**
 * ipc_subscribe() - Open a subscribe endpoint for a topic.
 * @id: topic identifier (see TopicId in ipc/topic.h).
 *
 * Attaches to the existing backing object created by the publisher; it does
 * not create one. The publisher of @id must already be running.
 *
 * Return: a subscriber handle on success, or NULL on failure (invalid topic
 * id, allocation failure, or transport open error).
 */
ipc_subscriber_t* ipc_subscribe(TopicId id);

/**
 * ipc_pull() - Pull the next message from a subscribe endpoint.
 * @sub:        subscriber handle from ipc_subscribe().
 * @buf:        destination buffer for the received message.
 * @size:       capacity of @buf in bytes; must be >= the topic's message size.
 * @timeout_ms: maximum time to wait for data, in milliseconds.
 *
 * For MQ topics this dequeues the oldest message in FIFO order. For SHM
 * topics it returns the latest value, but only if it is newer than the one
 * previously delivered to this subscriber (it polls until the deadline).
 *
 * Return: 0 if a message was copied into @buf, 1 on timeout / no new data,
 * -1 on error.
 */
int               ipc_pull(ipc_subscriber_t* sub, void* buf, size_t size, int timeout_ms);

/**
 * ipc_unsubscribe() - Close a subscribe endpoint.
 * @sub: subscriber handle, or NULL (no-op).
 *
 * Detaches from the transport and frees the handle. Does not unlink the
 * backing object; that is the publisher's responsibility.
 */
void              ipc_unsubscribe(ipc_subscriber_t* sub);

#ifdef __cplusplus
}
#endif
