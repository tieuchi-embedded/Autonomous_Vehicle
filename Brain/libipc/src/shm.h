#pragma once
#include "ipc/topic.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    void*    map;
    size_t   map_size;
    int      fd;
    uint32_t slot_size;
    uint16_t n_slots;
} ShmHandle;

int  shm_open_pub(TopicId id, size_t payload_size, ShmHandle* out);
int  shm_open_sub(TopicId id, ShmHandle* out);
int  shm_write(ShmHandle* h, const void* data, size_t size);
int  shm_read(ShmHandle* h, void* buf, size_t size, uint32_t* last_seq);
void shm_close(ShmHandle* h);
void shm_unlink_topic(TopicId id);
