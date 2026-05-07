#pragma once

namespace control {

// PID speed controller — returns motor RPM.
class SpeedController {
public:
    SpeedController(float kp = 10.0f, float ki = 0.5f, float kd = 0.1f);
    // target_speed: m/s, current_speed: cm/s (from EgoState)
    float compute(float target_speed_ms, float current_speed_cms, float dt_s);
    void  reset();

private:
    float m_kp, m_ki, m_kd;
    float m_integral{0.0f};
    float m_prev_err{0.0f};
    float m_max_rpm{300.0f};
};

} // namespace control
