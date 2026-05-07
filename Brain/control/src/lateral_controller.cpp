#include "control/lateral_controller.hpp"
#include <algorithm>

namespace control {

LateralController::LateralController(float lookahead_m)
    : m_lookahead(lookahead_m) {}

float LateralController::compute(float target_heading_deg) const {
    // Stub: pass-through with clamp until Pure Pursuit is tuned on real hardware
    return std::clamp(target_heading_deg, -m_max_steer, m_max_steer);
}

} // namespace control
