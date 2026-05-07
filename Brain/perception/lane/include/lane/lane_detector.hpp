#pragma once
#include <opencv2/core.hpp>

namespace lane {

enum class LaneSource { NONE, RIGHT, LEFT };

struct DetectResult {
    bool       ok  = false;
    LaneSource src = LaneSource::NONE;

    float heading_err_rad  = 0.0f;
    float lateral_offset_m = 0.0f;

    bool  ok_right = false;
    bool  ok_left  = false;

    // Line coefficients x = b*y + c for each lane (if detected)
    float rb = 0, rc = 0;  // right lane
    float lb = 0, lc = 0;  // left lane

    // Pixel coords on warped image (debug overlay)
    int   car_x  = 0;
    int   lane_x = 0;
    int   img_h  = 0;
    int   img_w  = 0;
};

// Run sliding window + polynomial fit on a binary bird's-eye image.
// Returns per-lane ok flags; heading computed from right lane (preferred),
// falls back to left if right unavailable.
DetectResult detect(const cv::Mat& binary_warped);

} // namespace lane
