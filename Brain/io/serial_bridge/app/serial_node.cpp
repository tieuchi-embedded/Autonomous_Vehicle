// serial_node — reads UART from STM32, publishes SERIAL topic.
// Phase 1: uplink only (@IMU and @RPM frames).
// Phase 3: add downlink (SerialWriter + CONTROL_CMD sub).

#include "ipc/bus.hpp"
#include "serial/protocol.hpp"
#include "serial/serial_reader.hpp"
#include "serial_data.h"

#include <atomic>
#include <csignal>
#include <cstdio>

static constexpr const char* DEVICE = "/dev/ttyACM0";
static constexpr int         BAUD   = 115200;

static std::atomic<bool> g_run{true};
static void on_signal(int) { g_run.store(false); }

int main(int argc, char* argv[]) {
    std::signal(SIGINT,  on_signal);
    std::signal(SIGTERM, on_signal);

    const char* device = (argc > 1) ? argv[1] : DEVICE;
    serial::SerialReader reader(device, BAUD);

    ipc::Publisher<SerialData> serial_pub(SERIAL);
    if (!serial_pub.valid()) {
        std::fprintf(stderr, "serial_node: ipc_publish_open failed\n");
        return 1;
    }

    std::printf("serial_node: listening on %s @ %d\n", device, BAUD);

    SerialData data{};

    while (g_run.load()) {
        auto line = reader.read_line();
        if (!line) continue;

        if (auto imu = serial::parse_imu(*line)) {
            data.yaw   = imu->yaw;
            data.pitch = imu->pitch;
            data.roll  = imu->roll;
            serial_pub.send(data);
            continue;
        }

        if (auto rpm = serial::parse_rpm(*line)) {
            data.rpm = rpm->rpm;
            serial_pub.send(data);
            continue;
        }
    }

    std::printf("serial_node: shutting down\n");
    return 0;
}
