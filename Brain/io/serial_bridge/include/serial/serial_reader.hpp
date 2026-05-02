#pragma once
#include <string>
#include <optional>

namespace serial {

// Blocking line reader over a UART fd.
// Opens device at construction, closes at destruction.
class SerialReader {
public:
    // Opens device (e.g. "/dev/ttyAMA0") at given baud rate.
    // Throws std::runtime_error on failure.
    SerialReader(const std::string& device, int baud);
    ~SerialReader();

    SerialReader(const SerialReader&)            = delete;
    SerialReader& operator=(const SerialReader&) = delete;

    // Read one line terminated by \n (strips \r\n).
    // Returns nullopt on timeout (1s) or read error.
    std::optional<std::string> read_line();

    // Expose fd for shared use by SerialWriter (same UART, full-duplex).
    int fd() const { return fd_; }

private:
    int fd_ = -1;
};

} // namespace serial
