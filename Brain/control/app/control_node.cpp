// control_node — sub BEHAVIOR_CMD + EGO_STATE, compute ControlCmd, pub CONTROL_CMD.

#include "ipc/bus.hpp"
#include "control/speed_controller.hpp"
#include "behavior_cmd.h"
#include "ego_state.h"
#include "control_cmd.h"

#include <algorithm>
#include <atomic>
#include <csignal>
#include <cstdio>
#include <ctime>

static std::atomic<bool> g_run{true};
static void on_signal(int) { g_run.store(false); }

static float now_s() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9f;
}

int main() {
    std::signal(SIGINT,  on_signal);
    std::signal(SIGTERM, on_signal);

    ipc::Subscriber<BehaviorCmd> behavior_sub(BEHAVIOR_CMD);
    ipc::Subscriber<EgoState>    ego_sub(EGO_STATE);
    ipc::Publisher<ControlCmd>   ctrl_pub(CONTROL_CMD);

    if (!behavior_sub.valid()) {
        std::fprintf(stderr, "control_node: subscribe BEHAVIOR_CMD failed\n");
        return 1;
    }
    if (!ego_sub.valid()) {
        std::fprintf(stderr, "control_node: subscribe EGO_STATE failed\n");
        return 1;
    }
    if (!ctrl_pub.valid()) {
        std::fprintf(stderr, "control_node: publish CONTROL_CMD failed\n");
        return 1;
    }

    control::SpeedController speed;

    constexpr float kMaxSteerDeg = 30.0f;  // matches control_cmd.h spec range

    float last_t = now_s();
    float current_speed_cms = 0.0f;
    float target_speed_ms   = 0.0f;
    float target_steer_deg  = 0.0f;

    std::printf("control_node: running (BEHAVIOR_CMD + EGO_STATE -> CONTROL_CMD)\n");

    while (g_run.load()) {
        auto beh = behavior_sub.poll(20);
        if (beh) {
            target_speed_ms  = beh->target_speed;
            target_steer_deg = beh->target_steer_deg;
        }

        auto ego = ego_sub.poll(0);
        if (ego) current_speed_cms = ego->speed;

        float now = now_s();
        float dt  = now - last_t;
        last_t    = now;

        ControlCmd cmd{};
        cmd.steer_deg = std::clamp(target_steer_deg, -kMaxSteerDeg, kMaxSteerDeg);
        cmd.rpm       = speed.compute(target_speed_ms, current_speed_cms, dt);
        ctrl_pub.send(cmd);

        std::printf("\rtarget_steer=%+6.2f deg  speed=%.2f cm/s  -> steer=%+6.2f deg  rpm=%+6.1f   ",
                    target_steer_deg, current_speed_cms, cmd.steer_deg, cmd.rpm);
        std::fflush(stdout);
    }

    std::printf("control_node: shutting down\n");
    return 0;
}
