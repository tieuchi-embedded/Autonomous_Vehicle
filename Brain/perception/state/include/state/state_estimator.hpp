#pragma once
#include "serial_data.h"
#include "ego_state.h"

#include <chrono>

namespace state {

class StateEstimator {
public:
    StateEstimator();

    void update(const SerialData& s);
    EgoState snapshot() const;

private:
    static constexpr float GEAR_RATIO      = 7.0f;
    static constexpr float WHEEL_RADIUS_CM = 6.5f;

    std::chrono::steady_clock::time_point start_;
    float angle_ = 0.0f;   // degrees
    float speed_ = 0.0f;   // cm/s
};

} // namespace state
