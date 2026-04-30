#include "shm.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// SHM layout: ShmHeader (32 bytes, cache-line friendly) + n_slots * slot_size
typedef struct {
    _Atomic uint32_t write_seq;
    uint32_t         slot_size;
    uint16_t         n_slots;
    uint8_t          _pad[22];
} ShmHeader;

#define SHM_HDR_SIZE sizeof(ShmHeader)

static int do_open(TopicId id, size_t payload_size, int create, ShmHandle* out) {
    const TopicDescriptor* td = topic_get(id);
    if (!td) return -1;

    size_t slot_size = payload_size;
    uint16_t n_slots = (uint16_t)td->depth;
    size_t map_size  = SHM_HDR_SIZE + (size_t)n_slots * slot_size;

    int flags = create ? (O_CREAT | O_RDWR) : O_RDWR;
    int fd = shm_open(td->name, flags, 0600);
    if (fd < 0) return -1;

    if (create && ftruncate(fd, (off_t)map_size) < 0) {
        close(fd);
        return -1;
    }

    void* map = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) { close(fd); return -1; }

    if (create) {
        ShmHeader* hdr = (ShmHeader*)map;
        hdr->write_seq = 0;
        hdr->slot_size = (uint32_t)slot_size;
        hdr->n_slots   = n_slots;
    } else {
        ShmHeader* hdr = (ShmHeader*)map;
        slot_size = hdr->slot_size;
        n_slots   = hdr->n_slots;
    }

    out->map       = map;
    out->map_size  = map_size;
    out->fd        = fd;
    out->slot_size = (uint32_t)slot_size;
    out->n_slots   = n_slots;
    return 0;
}

int shm_open_pub(TopicId id, size_t payload_size, ShmHandle* out) {
    return do_open(id, payload_size, 1, out);
}

int shm_open_sub(TopicId id, ShmHandle* out) {
    const TopicDescriptor* td = topic_get(id);
    if (!td) return -1;

    // Open or create — sub doesn't know payload_size yet, read it from header
    int fd = shm_open(td->name, O_CREAT | O_RDWR, 0600);
    if (fd < 0) return -1;

    // Map just the header first to read slot_size and n_slots
    size_t hdr_size = SHM_HDR_SIZE;
    if (ftruncate(fd, (off_t)hdr_size) < 0 && errno != EINVAL) {
        // EINVAL = already larger, which is fine
        close(fd); return -1;
    }
    ShmHeader* hdr = mmap(NULL, hdr_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (hdr == MAP_FAILED) { close(fd); return -1; }

    // Wait until pub initializes the header
    while (hdr->slot_size == 0 || hdr->n_slots == 0) {
        struct timespec ts = { .tv_sec = 0, .tv_nsec = 10000000L }; // 10ms
        nanosleep(&ts, NULL);
    }

    uint32_t slot_size = hdr->slot_size;
    uint16_t n_slots   = hdr->n_slots;
    munmap(hdr, hdr_size);

    size_t map_size = SHM_HDR_SIZE + (size_t)n_slots * slot_size;
    void* map = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) { close(fd); return -1; }

    out->map       = map;
    out->map_size  = map_size;
    out->fd        = fd;
    out->slot_size = slot_size;
    out->n_slots   = n_slots;
    return 0;
}

int shm_write(ShmHandle* h, const void* data, size_t size) {
    ShmHeader* hdr = (ShmHeader*)h->map;
    uint32_t seq  = __atomic_load_n(&hdr->write_seq, __ATOMIC_RELAXED);
    uint32_t slot = seq % h->n_slots;
    uint8_t* dst  = (uint8_t*)h->map + SHM_HDR_SIZE + slot * h->slot_size;
    size_t   copy = size < h->slot_size ? size : h->slot_size;
    memcpy(dst, data, copy);
    __atomic_fetch_add(&hdr->write_seq, 1, __ATOMIC_RELEASE);
    return 0;
}

// returns 1 if new data available, 0 if no new data
int shm_read(ShmHandle* h, void* buf, size_t size, uint32_t* last_seq) {
    ShmHeader* hdr     = (ShmHeader*)h->map;
    uint32_t   cur_seq = __atomic_load_n(&hdr->write_seq, __ATOMIC_ACQUIRE);
    if (cur_seq == *last_seq) return 0;
    uint32_t slot = (cur_seq - 1) % h->n_slots;
    uint8_t* src  = (uint8_t*)h->map + SHM_HDR_SIZE + slot * h->slot_size;
    size_t   copy = size < h->slot_size ? size : h->slot_size;
    memcpy(buf, src, copy);
    *last_seq = cur_seq;
    return 1;
}

void shm_close(ShmHandle* h) {
    if (h->map && h->map != MAP_FAILED) {
        munmap(h->map, h->map_size);
        h->map = NULL;
    }
    if (h->fd >= 0) {
        close(h->fd);
        h->fd = -1;
    }
}

void shm_unlink_topic(TopicId id) {
    const TopicDescriptor* td = topic_get(id);
    if (td) shm_unlink(td->name);
}
