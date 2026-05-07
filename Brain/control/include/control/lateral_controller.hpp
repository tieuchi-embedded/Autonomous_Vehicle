#pragma once

namespace control {

// Pure Pursuit lateral controller — returns steering angle in degrees.
class LateralController {
public:
    explicit LateralController(float lookahead_m = 0.3f);
    // target_heading_deg: desired heading change (from planning)
    // Returns steer_deg clamped to [-max_steer, max_steer]
    float compute(float target_heading_deg) const;

private:
    float m_lookahead;
    float m_max_steer{25.0f};  // degrees
};

} // namespace control
