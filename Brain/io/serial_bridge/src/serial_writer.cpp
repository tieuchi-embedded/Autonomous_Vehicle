#include "serial/serial_writer.hpp"

#include <cstdio>
#include <cstring>
#include <unistd.h>

namespace serial {

static void write_frame(int fd, const char* cmd, float val) {
    if (fd < 0) return;
    char buf[64];
    int n = std::snprintf(buf, sizeof(buf), "#%s:%.2f;;\r\n", cmd, val);
    if (n > 0) ::write(fd, buf, (size_t)n);
}

void SerialWriter::send_rpm(float rpm)   { write_frame(fd_, "RPM", rpm); }
void SerialWriter::send_steer(float deg) { write_frame(fd_, "STR", deg); }

} // namespace serial
