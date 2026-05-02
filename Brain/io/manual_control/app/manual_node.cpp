// manual_node — keyboard control over SSH terminal.
// Pub CONTROL_CMD; serial_node forwards to STM32.
//
// Keys (no Enter required):
//   w/s : rpm +/- step
//   a/d : steer left/right step
//   space : full stop (rpm=0, steer=0)
//   x : center steering only
//   q : quit
//
// Step sizes can be tuned with env vars MANUAL_RPM_STEP / MANUAL_STEER_STEP.

#include "ipc/bus.hpp"
#include "control_cmd.h"

#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

static std::atomic<bool> g_run{true};
static struct termios g_orig_tty;
static bool g_tty_saved = false;

static void restore_tty() {
    if (g_tty_saved) tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_tty);
}

static void on_signal(int) {
    g_run.store(false);
}

static bool set_raw_tty() {
    if (tcgetattr(STDIN_FILENO, &g_orig_tty) != 0) return false;
    g_tty_saved = true;

    struct termios raw = g_orig_tty;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;
    return tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0;
}

static int read_key_nonblock() {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    struct timeval tv = {0, 50 * 1000};  // 50 ms
    if (select(STDIN_FILENO + 1, &rfds, nullptr, nullptr, &tv) <= 0) return -1;
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;
    return (unsigned char)c;
}

static float env_float(const char* name, float fallback) {
    const char* s = std::getenv(name);
    if (!s) return fallback;
    char* end;
    float v = std::strtof(s, &end);
    return (end == s) ? fallback : v;
}

int main() {
    std::signal(SIGINT,  on_signal);
    std::signal(SIGTERM, on_signal);
    std::atexit(restore_tty);

    if (!set_raw_tty()) {
        std::fprintf(stderr, "manual_node: cannot set raw tty (need real TTY, not pipe)\n");
        return 1;
    }

    ipc::Publisher<ControlCmd> pub(CONTROL_CMD);
    if (!pub.valid()) {
        std::fprintf(stderr, "manual_node: ipc_publish_open failed\n");
        return 1;
    }

    const float RPM_STEP   = env_float("MANUAL_RPM_STEP",   20.0f);
    const float STEER_STEP = env_float("MANUAL_STEER_STEP", 5.0f);
    const float STEER_MAX  = env_float("MANUAL_STEER_MAX",  25.0f);

    float rpm   = 0.0f;
    float steer = 0.0f;
    bool  dirty = true;

    std::printf("manual_node: w/s=rpm  a/d=steer  space=stop  x=center  q=quit\n");
    std::printf("step rpm=%.1f steer=%.1f deg, max steer=%.1f deg\n",
                RPM_STEP, STEER_STEP, STEER_MAX);
    std::fflush(stdout);

    while (g_run.load()) {
        int key = read_key_nonblock();
        switch (key) {
            case 'w': rpm += RPM_STEP;   dirty = true; break;
            case 's': rpm -= RPM_STEP;   dirty = true; break;
            case 'a': steer -= STEER_STEP; dirty = true; break;
            case 'd': steer += STEER_STEP; dirty = true; break;
            case ' ': rpm = 0; steer = 0; dirty = true; break;
            case 'x': steer = 0;           dirty = true; break;
            case 'q': g_run.store(false);  break;
            default:  break;
        }

        if (steer >  STEER_MAX) steer =  STEER_MAX;
        if (steer < -STEER_MAX) steer = -STEER_MAX;

        if (dirty) {
            ControlCmd cmd{};
            cmd.rpm       = rpm;
            cmd.steer_deg = steer;
            pub.send(cmd);
            std::printf("\rrpm=%+7.1f  steer=%+6.1f deg   ", rpm, steer);
            std::fflush(stdout);
            dirty = false;
        }
    }

    // Safety stop
    ControlCmd stop{};
    pub.send(stop);

    std::printf("\nmanual_node: shutting down\n");
    return 0;
}
