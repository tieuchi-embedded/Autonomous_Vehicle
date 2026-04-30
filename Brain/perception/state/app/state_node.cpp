// state_node — sub SERIAL, compute EGO_STATE (yaw + speed), pub EGO_STATE.

#include "ipc/bus.hpp"
#include "state/state_estimator.hpp"
#include "serial_data.h"
#include "ego_state.h"

#include <atomic>
#include <csignal>
#include <cstdio>

static std::atomic<bool> g_run{true};
static void on_signal(int) { g_run.store(false); }

int main() {
    std::signal(SIGINT,  on_signal);
    std::signal(SIGTERM, on_signal);

    ipc::Subscriber<SerialData> serial_sub(SERIAL);
    if (!serial_sub.valid()) {
        std::fprintf(stderr, "state_node: subscribe SERIAL failed\n");
        return 1;
    }

    ipc::Publisher<EgoState> ego_pub(EGO_STATE);
    if (!ego_pub.valid()) {
        std::fprintf(stderr, "state_node: publish EGO_STATE failed\n");
        return 1;
    }

    state::StateEstimator est;
    std::printf("state_node: running (SERIAL -> EGO_STATE)\n");

    while (g_run.load()) {
        auto s = serial_sub.poll(500);
        if (!s) continue;

        est.update(*s);
        ego_pub.send(est.snapshot());
    }

    std::printf("state_node: shutting down\n");
    return 0;
}
