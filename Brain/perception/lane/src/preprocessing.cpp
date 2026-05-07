#include "lane/preprocessing.hpp"
#include <opencv2/imgproc.hpp>

namespace lane {

// IPM source trapezoid — calibrated from 320x240 frame of Full_map.mp4.
// Horizon ~35%, bottom crop ~80% (excludes wheel/chassis).
// Points: top-left, top-right, bot-right, bot-left (fraction of w/h).
static constexpr float SRC_TL_X = 0.2f;
static constexpr float SRC_TR_X = 0.8f;
static constexpr float SRC_TOP_Y = 0.50f;

static constexpr float SRC_BL_X = 0.00f;
static constexpr float SRC_BR_X = 1.00f;
static constexpr float SRC_BOT_Y = 0.80f;

// HLS threshold — white lane on dark asphalt.
// Only L channel: white markings are very bright (L>200), dark road is low.
static constexpr int L_MIN = 200;
static constexpr int L_MAX = 255;

cv::Mat ipm_matrix(int w, int h) {
    cv::Point2f src[4] = {
        {w * SRC_TL_X, h * SRC_TOP_Y},  // top-left
        {w * SRC_TR_X, h * SRC_TOP_Y},  // top-right
        {w * SRC_BR_X, h * SRC_BOT_Y},  // bot-right
        {w * SRC_BL_X, h * SRC_BOT_Y},  // bot-left
    };

    cv::Point2f dst[4] = {
        {0.0f,     0.0f},
        {(float)w, 0.0f},
        {(float)w, (float)h},
        {0.0f,     (float)h},
    };

    return cv::getPerspectiveTransform(src, dst);
}

std::array<cv::Point2f, 4> ipm_src_points(int w, int h) {
    return {{
        {w * SRC_TL_X, h * SRC_TOP_Y},
        {w * SRC_TR_X, h * SRC_TOP_Y},
        {w * SRC_BR_X, h * SRC_BOT_Y},
        {w * SRC_BL_X, h * SRC_BOT_Y},
    }};
}

cv::Mat ipm(const cv::Mat& src, const cv::Mat& M) {
    cv::Mat out;
    cv::warpPerspective(src, out, M, src.size(), cv::INTER_LINEAR);
    return out;
}

cv::Mat binary_mask(const cv::Mat& bgr) {
    cv::Mat hls, out;
    cv::cvtColor(bgr, hls, cv::COLOR_BGR2HLS);

    std::vector<cv::Mat> ch;
    cv::split(hls, ch);  // ch[1] = L

    cv::inRange(ch[1], L_MIN, L_MAX, out);
    return out;
}

} // namespace lane
