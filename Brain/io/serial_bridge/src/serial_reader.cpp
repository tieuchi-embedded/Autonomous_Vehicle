#include "serial/serial_reader.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

namespace serial {

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

SerialReader::SerialReader(const std::string& device, int baud) {
    fd_ = ::open(device.c_str(), O_RDWR | O_NOCTTY);
    if (fd_ < 0)
        throw std::runtime_error("open " + device + ": " + std::strerror(errno));

    termios tty{};
    if (::tcgetattr(fd_, &tty) != 0)
        throw std::runtime_error("tcgetattr: " + std::string(std::strerror(errno)));

    speed_t spd = to_baud(baud);
    ::cfsetispeed(&tty, spd);
    ::cfsetospeed(&tty, spd);
    ::cfmakeraw(&tty);

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 10;  // 1s timeout (tenths of seconds)

    if (::tcsetattr(fd_, TCSANOW, &tty) != 0)
        throw std::runtime_error("tcsetattr: " + std::string(std::strerror(errno)));
}

SerialReader::~SerialReader() {
    if (fd_ >= 0) ::close(fd_);
}

std::optional<std::string> SerialReader::read_line() {
    std::string line;
    line.reserve(64);

    while (true) {
        char c;
        ssize_t n = ::read(fd_, &c, 1);
        if (n < 0)  return std::nullopt;  // read error
        if (n == 0) return std::nullopt;  // timeout
        if (c == '\r') continue;          // strip CR
        if (c == '\n') return line;       // end of line
        line += c;
    }
}

} // namespace serial
