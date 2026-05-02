#pragma once

namespace serial {

// Downlink writer — sends control frames to STM32 over UART.
// Format: #RPM:value;;\r\n   #STR:value;;\r\n
class SerialWriter {
public:
    explicit SerialWriter(int fd) : fd_(fd) {}

    void send_rpm(float rpm);
    void send_steer(float deg);

private:
    int fd_ = -1;
};

} // namespace serial
