#pragma once
#include <string>

namespace serial {

// Downlink writer — Phase 3.
// Sends control commands to STM32 over UART.
// Format: #SPED::<float>\r\n  #STER::<float>\r\n  #BRAK::<float>\r\n
class SerialWriter {
public:
    explicit SerialWriter(int fd);

    void send_speed(float v);
    void send_steer(float angle);
    void send_brake(float pct);

private:
    int fd_ = -1;
    void write_cmd(const char* cmd, float val);
};

} // namespace serial
