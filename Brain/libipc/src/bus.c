#include "ipc/bus.h"
#include "mq_transport.h"
#include "shm.h"
#include <stdlib.h>
#include <time.h>

struct ipc_publisher {
    TopicId              id;
    TransportKind        transport;
    union {
        MqHandle mq;
        ShmHandle shm;
    };
};

struct ipc_subscriber {
    TopicId              id;
    TransportKind        transport;
    uint32_t             last_seq;  // SHM only
    union {
        MqHandle mq;
        ShmHandle shm;
    };
};

ipc_publisher_t* ipc_publish_open(TopicId id, size_t payload_size) {
    const TopicDescriptor* td = topic_get(id);
    if (!td) return NULL;

    ipc_publisher_t* pub = malloc(sizeof(*pub));
    if (!pub) return NULL;
    pub->id        = id;
    pub->transport = td->transport;

    int r;
    if (td->transport == SHM)
        r = shm_open_pub(id, payload_size, &pub->shm);
    else
        r = mq_transport_open_pub(id, &pub->mq);

    if (r != 0) { free(pub); return NULL; }
    return pub;
}

int ipc_publish(ipc_publisher_t* pub, const void* msg, size_t size) {
    if (pub->transport == SHM)
        return shm_write(&pub->shm, msg, size);
    const TopicDescriptor* td = topic_get(pub->id);
    return mq_transport_send(&pub->mq, td, msg, size);
}

void ipc_publish_close(ipc_publisher_t* pub) {
    if (!pub) return;
    if (pub->transport == SHM) shm_close(&pub->shm);
    else mq_transport_close(&pub->mq);
    free(pub);
}

ipc_subscriber_t* ipc_subscribe_open(TopicId id) {
    const TopicDescriptor* td = topic_get(id);
    if (!td) return NULL;

    ipc_subscriber_t* sub = malloc(sizeof(*sub));
    if (!sub) return NULL;
    sub->id        = id;
    sub->transport = td->transport;
    sub->last_seq  = 0;

    int r;
    if (td->transport == SHM)
        r = shm_open_sub(id, &sub->shm);
    else
        r = mq_transport_open_sub(id, &sub->mq);

    if (r != 0) { free(sub); return NULL; }
    return sub;
}

int ipc_poll(ipc_subscriber_t* sub, void* buf, size_t size, int timeout_ms) {
    if (sub->transport == MQ)
        return mq_transport_recv(&sub->mq, buf, size, timeout_ms);

    // SHM: spin-poll with 1ms sleep until timeout
    struct timespec deadline;
    clock_gettime(CLOCK_MONOTONIC, &deadline);
    deadline.tv_sec  += timeout_ms / 1000;
    deadline.tv_nsec += (long)(timeout_ms % 1000) * 1000000L;
    if (deadline.tv_nsec >= 1000000000L) {
        deadline.tv_sec++;
        deadline.tv_nsec -= 1000000000L;
    }

    while (1) {
        if (shm_read(&sub->shm, buf, size, &sub->last_seq)) return 0;
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (now.tv_sec > deadline.tv_sec ||
            (now.tv_sec == deadline.tv_sec && now.tv_nsec >= deadline.tv_nsec))
            return 1;
        struct timespec sleep_ts = { .tv_sec = 0, .tv_nsec = 1000000L }; // 1ms
        nanosleep(&sleep_ts, NULL);
    }
}

void ipc_subscribe_close(ipc_subscriber_t* sub) {
    if (!sub) return;
    if (sub->transport == SHM) shm_close(&sub->shm);
    else mq_transport_close(&sub->mq);
    free(sub);
}
