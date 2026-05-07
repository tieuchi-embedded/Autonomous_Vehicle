#include "control/speed_controller.hpp"
#include <algorithm>

namespace control {

SpeedController::SpeedController(float kp, float ki, float kd)
    : m_kp(kp), m_ki(ki), m_kd(kd) {}

float SpeedController::compute(float target_speed_ms, float current_speed_cms, float dt_s) {
    float current_ms = current_speed_cms / 100.0f;
    float err = target_speed_ms - current_ms;
    m_integral  += err * dt_s;
    float deriv  = (dt_s > 0.0f) ? (err - m_prev_err) / dt_s : 0.0f;
    m_prev_err   = err;
    float out    = m_kp * err + m_ki * m_integral + m_kd * deriv;
    return std::clamp(out, -m_max_rpm, m_max_rpm);
}

void SpeedController::reset() {
    m_integral = 0.0f;
    m_prev_err = 0.0f;
}

} // namespace control
