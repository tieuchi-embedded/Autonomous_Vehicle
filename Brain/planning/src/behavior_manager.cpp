#include "planning/behavior_manager.hpp"
#include <cstring>

namespace planning {

BehaviorManager::BehaviorManager() = default;

void BehaviorManager::update_lane(const LaneState& lane) {
    // Use heading error to drive target_heading setpoint
    // Convert rad to degrees for control layer
    m_target_heading = lane.heading_err_rad * (180.0f / 3.14159265f);
}

void BehaviorManager::update_ego(const EgoState& /*ego*/) {
    // Placeholder — extend for speed-based decisions
}

BehaviorCmd BehaviorManager::snapshot() const {
    BehaviorCmd cmd{};
    cmd.target_speed   = m_target_speed;
    cmd.target_heading = m_target_heading;
    cmd.mode           = static_cast<uint8_t>(m_mode);
    return cmd;
}

} // namespace planning
