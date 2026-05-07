// planning_node — sub LANE_STATE + EGO_STATE, compute BehaviorCmd, pub BEHAVIOR_CMD.

#include "ipc/bus.hpp"
#include "planning/behavior_manager.hpp"
#include "lane_state.h"
#include "ego_state.h"
#include "behavior_cmd.h"

#include <atomic>
#include <csignal>
#include <cstdio>

static std::atomic<bool> g_run{true};
static void on_signal(int) { g_run.store(false); }

int main() {
    std::signal(SIGINT,  on_signal);
    std::signal(SIGTERM, on_signal);

    ipc::Subscriber<LaneState>  lane_sub(LANE_STATE);
    ipc::Subscriber<EgoState>   ego_sub(EGO_STATE);
    ipc::Publisher<BehaviorCmd> behavior_pub(BEHAVIOR_CMD);

    if (!lane_sub.valid()) {
        std::fprintf(stderr, "planning_node: subscribe LANE_STATE failed\n");
        return 1;
    }
    if (!ego_sub.valid()) {
        std::fprintf(stderr, "planning_node: subscribe EGO_STATE failed\n");
        return 1;
    }
    if (!behavior_pub.valid()) {
        std::fprintf(stderr, "planning_node: publish BEHAVIOR_CMD failed\n");
        return 1;
    }

    planning::BehaviorManager mgr;
    std::printf("planning_node: running (LANE_STATE + EGO_STATE -> BEHAVIOR_CMD)\n");

    while (g_run.load()) {
        auto lane = lane_sub.poll(50);
        if (lane) mgr.update_lane(*lane);

        auto ego = ego_sub.poll(0);
        if (ego) mgr.update_ego(*ego);

        if (lane) behavior_pub.send(mgr.snapshot());
    }

    std::printf("planning_node: shutting down\n");
    return 0;
}
