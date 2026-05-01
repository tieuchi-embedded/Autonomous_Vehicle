#include "lane/lane_tracker.hpp"

namespace lane {

DetectResult Tracker::smooth(const DetectResult& cur) {
    if (!cur.ok) return prev_;

    if (!init_) {
        prev_ = cur;
        init_ = true;
        return cur;
    }

    DetectResult out  = cur;  // keep ok/ok_right/ok_left/src/car_x/lane_x/img_h from raw
    out.heading_err_rad  = ALPHA * prev_.heading_err_rad  + (1.0f - ALPHA) * cur.heading_err_rad;
    out.lateral_offset_m = ALPHA * prev_.lateral_offset_m + (1.0f - ALPHA) * cur.lateral_offset_m;
    prev_ = out;
    return out;
}

} // namespace lane
