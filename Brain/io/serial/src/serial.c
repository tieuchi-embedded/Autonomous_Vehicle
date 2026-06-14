#include "serial/serial.h"

#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

static speed_t to_baud(int baud) {
    switch (baud) {
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        default:     return B115200;
    }
}

int serial_open(const char* device, int baud) {
    int fd = open(device, O_RDWR | O_NOCTTY);
    if (fd < 0) return -1;

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) { close(fd); return -1; }

    speed_t spd = to_baud(baud);
    cfsetispeed(&tty, spd);
    cfsetospeed(&tty, spd);
    cfmakeraw(&tty);

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 1;   // 100ms read timeout (tenths of a second)

    if (tcsetattr(fd, TCSANOW, &tty) != 0) { close(fd); return -1; }
    return fd;
}

int serial_read_line(int fd, char* buf, size_t buflen) {
    size_t len = 0;
    while (1) {
        char c;
        ssize_t n = read(fd, &c, 1);
        if (n <= 0) return -1;             // timeout or read error
        if (c == '\r') continue;           // strip CR
        if (c == '\n') { buf[len] = '\0'; return (int)len; }
        if (len + 1 < buflen) buf[len++] = c;
    }
}

static void write_frame(int fd, const char* cmd, float val) {
    if (fd < 0) return;
    char buf[64];
    int n = snprintf(buf, sizeof(buf), "#%s:%.2f;;\r\n", cmd, val);
    if (n > 0) (void)!write(fd, buf, (size_t)n);
}

void serial_send_rpm(int fd, float rpm)   { write_frame(fd, "RPM", rpm); }
void serial_send_steer(int fd, float deg) { write_frame(fd, "STR", deg); }

void serial_close(int fd) {
    if (fd >= 0) close(fd);
}
