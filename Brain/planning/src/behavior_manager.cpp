#include "planning/behavior_manager.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace planning {

namespace {
constexpr float kBaseSpeed       = 0.35f;  // m/s, straight-line target
constexpr float kReductionFactor = 0.5f;  // max fraction of speed shed in sharp turns
constexpr float kMaxHeadingErr   = 0.4f;  // rad — heading error considered a "sharp turn"

// Stanley lateral controller constants
constexpr float kStanleyGain = 6.0f;
constexpr float kMaxSteerDeg = 30.0f;     // matches control_cmd.h spec range
constexpr float kEpsilonMs   = 0.5f;      // m/s — avoids atan blow-up near zero speed
constexpr float kRadToDeg    = 180.0f / 3.14159265f;
}

BehaviorManager::BehaviorManager() = default;

void BehaviorManager::update_lane(const LaneState& lane) {
    // Slow down proportionally to how sharp the upcoming turn is.
    float reduction = std::clamp(std::fabs(lane.heading_err_rad) / kMaxHeadingErr, 0.0f, 1.0f);
    m_target_speed = kBaseSpeed * (1.0f - kReductionFactor * reduction);

    // Stanley: steer = heading_error + atan(k * cross_track_error / speed)
    float steer_rad = -lane.heading_err_rad
        - std::atan2(kStanleyGain * lane.lateral_offset_m, m_speed_ms + kEpsilonMs);
    m_target_steer_deg = std::clamp(steer_rad * kRadToDeg, -kMaxSteerDeg, kMaxSteerDeg);
}

void BehaviorManager::update_ego(const EgoState& ego) {
    m_speed_ms = ego.speed / 100.0f;  // cm/s -> m/s
}

BehaviorCmd BehaviorManager::snapshot() const {
    BehaviorCmd cmd{};
    cmd.target_speed     = m_target_speed;
    cmd.target_steer_deg = m_target_steer_deg;
    cmd.mode             = static_cast<uint8_t>(m_mode);
    return cmd;
}

} // namespace planning
