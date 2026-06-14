
/* SPDX-License-Identifier: MIT */
/*
 * shm.h - Shared-memory transport for libipc (internal).
 *
 * Implements the SHM side of the bus: a latest-wins ring of fixed-size slots
 * backed by a POSIX shared-memory object. This is the right transport for
 * high-rate state (camera frames, ego state) where only the freshest sample
 * matters and a slow reader may skip intermediate values.
 *
 * Layout of the mapped segment:
 *
 *     +-----------------------+  offset 0
 *     | ShmHeader             |  atomic write_seq, slot_size, n_slots
 *     +-----------------------+  offset SHM_HDR_SIZE
 *     | slot[0]               |
 *     | slot[1]               |
 *     | ...                   |
 *     | slot[n_slots-1]       |
 *     +-----------------------+
 *
 * Synchronization is a single-producer / multi-consumer scheme using one
 * atomic counter (write_seq) in the header:
 *   - The publisher writes into slot (write_seq % n_slots), then bumps
 *     write_seq with release ordering.
 *   - A subscriber loads write_seq with acquire ordering and reads the most
 *     recent slot (write_seq - 1). It tracks the last sequence it saw, so it
 *     can tell whether a new sample is available.
 * There is no reader lock; with enough slots a reader observes a complete
 * frame before the producer wraps back to overwrite it.
 *
 * Ordering rule: only the publisher creates and sizes the segment. A
 * subscriber waits (without creating) until the publisher has created the
 * object and published its slot geometry, so the publisher MUST start first.
 *
 * This is an internal interface used by ipc.c. Applications should use the
 * public API in ipc/ipc.h and never call these functions directly.
 */
#pragma once
#include "ipc/topic.h"
#include <stddef.h>
#include <stdint.h>

/**
 * struct ShmHandle - Mapped shared-memory endpoint.
 * @map:       base address of the mapped segment (header + slots).
 * @map_size:  total mapped size in bytes.
 * @fd:        shared-memory file descriptor, or -1 when closed.
 * @slot_size: size of one slot in bytes.
 * @n_slots:   number of slots in the ring.
 */
typedef struct {
    void*    map;
    size_t   map_size;
    int      fd;
    uint32_t slot_size;
    uint16_t n_slots;
} ShmHandle;

/**
 * shm_publish() - Create and map a topic's shared-memory segment for writing.
 * @id:           topic identifier.
 * @payload_size: size of one slot in bytes (the publisher defines the geometry).
 * @out:          handle to initialize on success.
 *
 * Creates the segment, sizes it for @payload_size * depth slots, and writes
 * the slot geometry into the header so subscribers can map it.
 *
 * Return: 0 on success, -1 on error.
 */
int  shm_publish(TopicId id, size_t payload_size, ShmHandle* out);

/**
 * shm_subscribe() - Map an existing topic's shared-memory segment for reading.
 * @id:  topic identifier.
 * @out: handle to initialize on success.
 *
 * Does NOT create the segment. Blocks until the publisher has created it and
 * published a non-zero slot geometry, then maps it using that geometry.
 *
 * Return: 0 on success, -1 on error.
 */
int  shm_subscribe(TopicId id, ShmHandle* out);

/**
 * shm_push() - Publish one message into the next slot (latest-wins).
 * @h:    open publisher handle.
 * @data: message payload.
 * @size: byte size of @data; truncated to slot_size if larger.
 *
 * Copies into slot (write_seq % n_slots) and then advances write_seq with
 * release ordering to make the new sample visible to readers.
 *
 * Return: 0 on success.
 */
int  shm_push(ShmHandle* h, const void* data, size_t size);

/**
 * shm_pull() - Read the latest message if newer than the last one seen.
 * @h:        open subscriber handle.
 * @buf:      destination buffer.
 * @size:     capacity of @buf in bytes; copy is capped at slot_size.
 * @last_seq: in/out cursor of the last sequence delivered to this reader;
 *            updated to the current sequence when new data is returned.
 *
 * Acquire-loads write_seq and, if it differs from *@last_seq, copies the most
 * recent slot (write_seq - 1) into @buf.
 *
 * Return: 1 if a new message was copied, 0 if no new data is available.
 */
int  shm_pull(ShmHandle* h, void* buf, size_t size, uint32_t* last_seq);

/**
 * shm_disconnect() - Unmap and close a shared-memory handle.
 * @h: handle to close; safe to call more than once.
 *
 * Releases the mapping and descriptor but does not remove the object from
 * the system (see shm_remove()).
 */
void shm_disconnect(ShmHandle* h);

/**
 * shm_remove() - Remove a topic's shared-memory object from the system.
 * @id: topic identifier.
 *
 * Unlinks the named object so it is freed once all mappings are released.
 * Typically called by the publisher on shutdown.
 */
void shm_remove(TopicId id);
