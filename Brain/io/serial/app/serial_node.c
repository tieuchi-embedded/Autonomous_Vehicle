// serial_node — UART bridge to STM32.
// Uplink:   parse @IMU/@RPM -> push EGO_STATE (raw telemetry)
// Downlink: pull CONTROL_CMD -> write #RPM/#STR to UART

#include "ipc/bus.h"
#include "serial/serial.h"
#include "serial/protocol.h"
#include "ego_state.h"
#include "control_cmd.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>

#define DEVICE "/dev/ttyACM0"
#define BAUD   115200

static volatile sig_atomic_t g_run = 1;
static void on_signal(int s) { (void)s; g_run = 0; }

int main(int argc, char* argv[]) {
    signal(SIGINT,  on_signal);
    signal(SIGTERM, on_signal);

    const char* device = (argc > 1) ? argv[1] : DEVICE;

    int fd = serial_open(device, BAUD);
    if (fd < 0) {
        fprintf(stderr, "serial_node: cannot open %s\n", device);
        return 1;
    }

    ipc_publisher_t*  ego_push = ipc_publish(EGO_STATE, sizeof(EgoState));
    ipc_subscriber_t* cmd_pull = ipc_subscribe(CONTROL_CMD);
    if (!ego_push || !cmd_pull) {
        fprintf(stderr, "serial_node: ipc open failed\n");
        serial_close(fd);
        return 1;
    }

    printf("serial_node: %s @ %d (uplink+downlink)\n", device, BAUD);

    EgoState data = {0};

    while (g_run) {
        // Drain pending control commands (non-blocking)
        ControlCmd cmd;
        while (ipc_pull(cmd_pull, &cmd, sizeof(cmd), 0) == 0) {
            serial_send_rpm(fd, cmd.rpm);
            serial_send_steer(fd, cmd.steer_deg);
        }

        // Read one UART line (blocks up to read timeout)
        char line[128];
        if (serial_read_line(fd, line, sizeof(line)) < 0) continue;

        if (parse_imu(line, &data.yaw, &data.pitch, &data.roll)) {
            ipc_push(ego_push, &data, sizeof(data));
            continue;
        }

        if (parse_rpm(line, &data.rpm)) {
            ipc_push(ego_push, &data, sizeof(data));
            continue;
        }
    }

    // Safety: stop motor on shutdown
    serial_send_rpm(fd, 0.0f);
    serial_send_steer(fd, 0.0f);

    ipc_unpublish(ego_push);
    ipc_unsubscribe(cmd_pull);
    serial_close(fd);
    printf("serial_node: shutting down\n");
    return 0;
}
