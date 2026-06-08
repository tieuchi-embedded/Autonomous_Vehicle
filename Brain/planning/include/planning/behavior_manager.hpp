#pragma once
#include "lane_state.h"
#include "ego_state.h"
#include "behavior_cmd.h"

namespace planning {

enum class BehaviorMode : uint8_t {
    IDLE           = 0,
    LANE_FOLLOW    = 1,
    STOP           = 2,
    OBSTACLE_AVOID = 3,
};

class BehaviorManager {
public:
    BehaviorManager();
    void update_lane(const LaneState& lane);
    void update_ego(const EgoState& ego);
    BehaviorCmd snapshot() const;

private:
    BehaviorMode m_mode{BehaviorMode::LANE_FOLLOW};
    float m_target_speed{0.4f};
    float m_target_steer_deg{0.0f};
    float m_speed_ms{0.0f};  // current speed, from EgoState — needed by Stanley
};

} // namespace planning
