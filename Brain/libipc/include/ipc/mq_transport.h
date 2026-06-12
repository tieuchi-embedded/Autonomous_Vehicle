#pragma once
#include "ipc/topic.h"
#include <mqueue.h>

typedef struct { mqd_t mqd; } MqHandle;

int  mq_transport_open_pub(TopicId id, MqHandle* out);
int  mq_transport_open_sub(TopicId id, MqHandle* out);
int  mq_transport_send(MqHandle* h, const TopicDescriptor* td, const void* msg, size_t size);
int  mq_transport_recv(MqHandle* h, void* buf, size_t size, int timeout_ms);
void mq_transport_close(MqHandle* h);
void mq_transport_unlink(TopicId id);
