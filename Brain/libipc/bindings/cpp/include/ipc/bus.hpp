#pragma once
#include "ipc/bus.h"
#include <optional>
#include <cstddef>

namespace ipc {

template<typename T>
class Publisher {
    ipc_publisher_t* pub_;
public:
    explicit Publisher(TopicId id, size_t payload_size = sizeof(T))
        : pub_(ipc_publish_open(id, payload_size)) {}
    ~Publisher() { ipc_publish_close(pub_); }

    Publisher(const Publisher&)            = delete;
    Publisher& operator=(const Publisher&) = delete;

    bool valid() const { return pub_ != nullptr; }
    int send(const T& msg) { return ipc_publish(pub_, &msg, sizeof(T)); }
};

template<typename T>
class Subscriber {
    ipc_subscriber_t* sub_;
public:
    explicit Subscriber(TopicId id) : sub_(ipc_subscribe_open(id)) {}
    ~Subscriber() { ipc_subscribe_close(sub_); }

    Subscriber(const Subscriber&)            = delete;
    Subscriber& operator=(const Subscriber&) = delete;

    bool valid() const { return sub_ != nullptr; }

    // returns nullopt on timeout or error
    std::optional<T> poll(int timeout_ms = 100) {
        T buf{};
        return ipc_poll(sub_, &buf, sizeof(T), timeout_ms) == 0
            ? std::optional<T>(buf) : std::nullopt;
    }
};

} // namespace ipc
