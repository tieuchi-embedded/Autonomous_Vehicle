#pragma once
#include "lane/lane_detector.hpp"

namespace lane {

// Exponential moving average over consecutive DetectResults.
// On detect failure, holds previous values.
class Tracker {
public:
    DetectResult smooth(const DetectResult& cur);

private:
    static constexpr float ALPHA = 0.7f;  // weight for prev (vs 1-ALPHA for current)
    DetectResult prev_{};
    bool         init_ = false;
};

} // namespace lane
