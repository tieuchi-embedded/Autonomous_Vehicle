/* SPDX-License-Identifier: MIT */
/*
 * mq.c - POSIX message queue transport implementation. See ipc/mq.h.
 */
#define _POSIX_C_SOURCE 200809L
#include "ipc/mq.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* read_sys_limit() - Read a long from a sysfs/procfs file, or @fallback. */
static long read_sys_limit(const char* path, long fallback) {
    FILE* f = fopen(path, "r");
    if (!f) return fallback;
    long v = fallback;
    if (fscanf(f, "%ld", &v) != 1) v = fallback;
    fclose(f);
    return v;
}

/*
 * open_mq() - Common open path for both pub and sub.
 * Builds the mq_attr from the topic descriptor, clamping the requested depth
 * to the system's mqueue/msg_max, then mq_open()s the named queue with
 * @flags | O_CREAT.
 */
static int open_mq(TopicId id, int flags, MqHandle* out) {
    const TopicDescriptor* td = topic_get(id);
    if (!td) return -1;

    long msg_max = read_sys_limit("/proc/sys/fs/mqueue/msg_max", 10);
    long depth   = td->depth > msg_max ? msg_max : td->depth;

    struct mq_attr attr = {
        .mq_flags   = 0,
        .mq_maxmsg  = depth,
        .mq_msgsize = (long)td->payload_size,
        .mq_curmsgs = 0,
    };

    mqd_t mqd = mq_open(td->name, flags | O_CREAT, 0600, &attr);
    if (mqd == (mqd_t)-1) return -1;
    out->mqd = mqd;
    return 0;
}

int mq_publish(TopicId id, MqHandle* out) {
    return open_mq(id, O_WRONLY, out);
}

int mq_subscribe(TopicId id, MqHandle* out) {
    return open_mq(id, O_RDWR, out);
}

int mq_push(MqHandle* h, const TopicDescriptor* td, const void* msg, size_t size) {
    /* Latest-wins: if the queue is full, drop the oldest message(s) to make
     * room for the new one, then enqueue it at the tail. Never blocks. */
    while (mq_timedsend(h->mqd, msg, size, 0, &(struct timespec){0,0}) != 0) {
        if (errno != ETIMEDOUT && errno != EAGAIN) return -1;
        char dummy[td->payload_size];
        if (mq_receive(h->mqd, dummy, td->payload_size, NULL) < 0) return -1;
    }
    return 0;
}

int mq_pull(MqHandle* h, void* buf, size_t size, int timeout_ms) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec  += timeout_ms / 1000;
    ts.tv_nsec += (long)(timeout_ms % 1000) * 1000000L;
    if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000L;
    }
    ssize_t r = mq_timedreceive(h->mqd, buf, size, NULL, &ts);
    if (r >= 0) return 0;
    return (errno == ETIMEDOUT || errno == EAGAIN) ? 1 : -1;
}

void mq_disconnect(MqHandle* h) {
    if (h->mqd != (mqd_t)-1) {
        mq_close(h->mqd);
        h->mqd = (mqd_t)-1;
    }
}

void mq_remove(TopicId id) {
    const TopicDescriptor* td = topic_get(id);
    if (td) mq_unlink(td->name);
}
