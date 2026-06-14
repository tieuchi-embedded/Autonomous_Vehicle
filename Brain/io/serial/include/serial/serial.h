#pragma once

#include <stddef.h>

// UART port to the STM32, full-duplex over a single fd.
//   - serial_open() configures raw mode at the given baud.
//   - serial_read_line() reads one '\n'-terminated line (strips '\r').
//   - serial_send_rpm() / serial_send_steer() write control frames:
//       #RPM:value;;\r\n   #STR:value;;\r\n

#ifdef __cplusplus
extern "C" {
#endif

// Open and configure the UART. Return fd >= 0 on success, -1 on error.
int serial_open(const char* device, int baud);

// Read one line (without \r\n) into buf (capacity buflen).
// Return line length on success, -1 on timeout / read error.
int serial_read_line(int fd, char* buf, size_t buflen);

// Downlink control frames to the STM32.
void serial_send_rpm(int fd, float rpm);
void serial_send_steer(int fd, float deg);

void serial_close(int fd);

#ifdef __cplusplus
}
#endif
