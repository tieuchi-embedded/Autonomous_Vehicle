#define _POSIX_C_SOURCE 200809L
#include "ipc/bus.h"
#include "ipc/mq.h"
#include "ipc/shm.h"
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

ipc_publisher_t* ipc_publish(TopicId id, size_t payload_size) {
    const TopicDescriptor* td = topic_get(id);
    if (!td) return NULL;

    ipc_publisher_t* pub = malloc(sizeof(*pub));
    if (!pub) return NULL;
    pub->id        = id;
    pub->transport = td->transport;

    int r;
    if (td->transport == SHM)
        r = shm_publish(id, payload_size, &pub->shm);
    else
        r = mq_publish(id, &pub->mq);

    if (r != 0) { free(pub); return NULL; }
    return pub;
}

int ipc_push(ipc_publisher_t* pub, const void* msg, size_t size) {
    if (pub->transport == SHM)
        return shm_push(&pub->shm, msg, size);
    const TopicDescriptor* td = topic_get(pub->id);
    return mq_push(&pub->mq, td, msg, size);
}

void ipc_unpublish(ipc_publisher_t* pub) {
    if (!pub) return;
    if (pub->transport == SHM) {
        shm_disconnect(&pub->shm);
        shm_remove(pub->id);
    } else {
        mq_disconnect(&pub->mq);
        mq_remove(pub->id);
    }
    free(pub);
}

ipc_subscriber_t* ipc_subscribe(TopicId id) {
    const TopicDescriptor* td = topic_get(id);
    if (!td) return NULL;

    ipc_subscriber_t* sub = malloc(sizeof(*sub));
    if (!sub) return NULL;
    sub->id        = id;
    sub->transport = td->transport;
    sub->last_seq  = 0;

    int r;
    if (td->transport == SHM)
        r = shm_subscribe(id, &sub->shm);
    else
        r = mq_subscribe(id, &sub->mq);

    if (r != 0) { free(sub); return NULL; }
    return sub;
}

int ipc_pull(ipc_subscriber_t* sub, void* buf, size_t size, int timeout_ms) {
    if (sub->transport == MQ)
        return mq_pull(&sub->mq, buf, size, timeout_ms);

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
        if (shm_pull(&sub->shm, buf, size, &sub->last_seq)) return 0;
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (now.tv_sec > deadline.tv_sec ||
            (now.tv_sec == deadline.tv_sec && now.tv_nsec >= deadline.tv_nsec))
            return 1;
        struct timespec sleep_ts = { .tv_sec = 0, .tv_nsec = 1000000L }; // 1ms
        nanosleep(&sleep_ts, NULL);
    }
}

void ipc_unsubscribe(ipc_subscriber_t* sub) {
    if (!sub) return;
    if (sub->transport == SHM) shm_disconnect(&sub->shm);
    else mq_disconnect(&sub->mq);
    free(sub);
}
