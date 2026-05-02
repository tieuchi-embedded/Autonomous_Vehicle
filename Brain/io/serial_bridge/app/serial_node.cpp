// serial_node — UART bridge to STM32.
// Uplink:   parse @IMU/@RPM -> pub SERIAL
// Downlink: sub CONTROL_CMD -> write #RPM/#STR to UART

#include "ipc/bus.hpp"
#include "serial/protocol.hpp"
#include "serial/serial_reader.hpp"
#include "serial/serial_writer.hpp"
#include "serial_data.h"
#include "control_cmd.h"

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
    serial::SerialWriter writer(reader.fd());

    ipc::Publisher<SerialData>   serial_pub(SERIAL);
    ipc::Subscriber<ControlCmd>  cmd_sub(CONTROL_CMD);
    if (!serial_pub.valid() || !cmd_sub.valid()) {
        std::fprintf(stderr, "serial_node: ipc open failed\n");
        return 1;
    }

    std::printf("serial_node: %s @ %d (uplink+downlink)\n", device, BAUD);

    SerialData data{};

    while (g_run.load()) {
        // Drain any pending control commands (non-blocking)
        while (auto cmd = cmd_sub.poll(0)) {
            writer.send_rpm(cmd->rpm);
            writer.send_steer(cmd->steer_deg);
        }

        // Read one UART line (blocks up to 1s, see SerialReader VTIME)
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

    // Safety: stop motor on shutdown
    writer.send_rpm(0.0f);
    writer.send_steer(0.0f);

    std::printf("serial_node: shutting down\n");
    return 0;
}
