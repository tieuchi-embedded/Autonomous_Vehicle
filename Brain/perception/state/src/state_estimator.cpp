#include "state/state_estimator.hpp"
#include <cmath>

namespace state {

StateEstimator::StateEstimator()
    : start_(std::chrono::steady_clock::now()) {}

void StateEstimator::update(const SerialData& s) {
    angle_ = s.yaw;
    speed_ = (s.rpm / GEAR_RATIO) * (2.0f * static_cast<float>(M_PI)) * WHEEL_RADIUS_CM / 60.0f;
}

EgoState StateEstimator::snapshot() const {
    auto now = std::chrono::steady_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count();

    EgoState ego{};
    ego.time_ms = static_cast<uint32_t>(ms);
    ego.angle   = angle_;
    ego.speed   = speed_;
    return ego;
}

} // namespace state
