#pragma once
#include "ipc/topic.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ipc_publisher  ipc_publisher_t;
typedef struct ipc_subscriber ipc_subscriber_t;

ipc_publisher_t*  ipc_publish_open(TopicId id, size_t payload_size);
int               ipc_publish(ipc_publisher_t* pub, const void* msg, size_t size);
void              ipc_publish_close(ipc_publisher_t* pub);

ipc_subscriber_t* ipc_subscribe_open(TopicId id);
// returns 0=ok, 1=timeout/no-new-data, -1=err
int               ipc_poll(ipc_subscriber_t* sub, void* buf, size_t size, int timeout_ms);
void              ipc_subscribe_close(ipc_subscriber_t* sub);

#ifdef __cplusplus
}
#endif
