#pragma once
#include <opencv2/core.hpp>
#include <array>

namespace lane {

cv::Mat ipm_matrix(int w, int h);
cv::Mat ipm(const cv::Mat& src, const cv::Mat& M);
cv::Mat binary_mask(const cv::Mat& bgr);

// Returns the 4 source trapezoid points used by ipm_matrix (TL, TR, BR, BL).
std::array<cv::Point2f, 4> ipm_src_points(int w, int h);

} // namespace lane
