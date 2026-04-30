#include "mq_transport.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static long read_sys_limit(const char* path, long fallback) {
    FILE* f = fopen(path, "r");
    if (!f) return fallback;
    long v = fallback;
    fscanf(f, "%ld", &v);
    fclose(f);
    return v;
}

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

int mq_transport_open_pub(TopicId id, MqHandle* out) {
    return open_mq(id, O_WRONLY, out);
}

int mq_transport_open_sub(TopicId id, MqHandle* out) {
    return open_mq(id, O_RDWR, out);
}

int mq_transport_send(MqHandle* h, const TopicDescriptor* td, const void* msg, size_t size) {
    switch (td->drop) {
        case NEW: {
            if (mq_timedsend(h->mqd, msg, size, 0, &(struct timespec){0,0}) == 0) return 0;
            if (errno != ETIMEDOUT && errno != EAGAIN) return -1;
            char dummy[td->payload_size];
            mq_receive(h->mqd, dummy, td->payload_size, NULL);
            return mq_timedsend(h->mqd, msg, size, 0, &(struct timespec){0,0}) == 0 ? 0 : -1;
        }
        case OLD: {
            while (mq_timedsend(h->mqd, msg, size, 0, &(struct timespec){0,0}) != 0) {
                if (errno != ETIMEDOUT && errno != EAGAIN) return -1;
                char dummy[td->payload_size];
                if (mq_receive(h->mqd, dummy, td->payload_size, NULL) < 0) return -1;
            }
            return 0;
        }
        case NEVER: {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 5;
            return mq_timedsend(h->mqd, msg, size, 0, &ts) == 0 ? 0 : -1;
        }
    }
    return -1;
}

int mq_transport_recv(MqHandle* h, void* buf, size_t size, int timeout_ms) {
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

void mq_transport_close(MqHandle* h) {
    if (h->mqd != (mqd_t)-1) {
        mq_close(h->mqd);
        h->mqd = (mqd_t)-1;
    }
}

void mq_transport_unlink(TopicId id) {
    const TopicDescriptor* td = topic_get(id);
    if (td) mq_unlink(td->name);
}
