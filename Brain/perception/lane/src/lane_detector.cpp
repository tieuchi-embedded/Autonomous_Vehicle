#include "lane/lane_detector.hpp"
#include <opencv2/imgproc.hpp>
#include <cmath>

namespace lane {

static constexpr int   N_WINDOWS  = 9;
static constexpr int   MARGIN     = 40;
static constexpr int   MIN_PIX    = 30;
static constexpr float PIXEL_TO_M = 0.005f;

// Fit line x = b*y + c by least squares.
static bool linefit(const std::vector<int>& xs,
                    const std::vector<int>& ys,
                    float& b, float& c) {
    int n = (int)xs.size();
    if (n < 2) return false;

    cv::Mat A(n, 2, CV_32F), B(n, 1, CV_32F);
    for (int i = 0; i < n; ++i) {
        A.at<float>(i, 0) = (float)ys[i];
        A.at<float>(i, 1) = 1.0f;
        B.at<float>(i, 0) = (float)xs[i];
    }
    cv::Mat coef;
    if (!cv::solve(A, B, coef, cv::DECOMP_NORMAL | cv::DECOMP_SVD)) return false;
    b = coef.at<float>(0, 0);
    c = coef.at<float>(1, 0);
    return true;
}

static float lineval(float b, float c, float y) { return b * y + c; }

// Heading error = atan(slope) — slope of x=b*y+c vs vertical axis.
static float line_heading(float b) { return std::atan(b); }

DetectResult detect(const cv::Mat& bw) {
    DetectResult r;
    if (bw.empty()) return r;

    int h = bw.rows, w = bw.cols;

    // Histogram of bottom half to find lane bases
    cv::Mat bottom = bw(cv::Rect(0, h / 2, w, h / 2));
    cv::Mat colsum;
    cv::reduce(bottom, colsum, 0, cv::REDUCE_SUM, CV_32S);

    int mid = w / 2;
    int leftx_base = 0, rightx_base = w - 1;
    int max_left = -1, max_right = -1;
    for (int x = 0; x < mid; ++x) {
        int v = colsum.at<int>(0, x);
        if (v > max_left) { max_left = v; leftx_base = x; }
    }
    for (int x = mid; x < w; ++x) {
        int v = colsum.at<int>(0, x);
        if (v > max_right) { max_right = v; rightx_base = x; }
    }

    int win_h = h / N_WINDOWS;
    int leftx_cur  = leftx_base;
    int rightx_cur = rightx_base;

    std::vector<int> lx, ly, rx, ry;

    for (int w_i = 0; w_i < N_WINDOWS; ++w_i) {
        int y_lo = h - (w_i + 1) * win_h;
        int y_hi = h -  w_i      * win_h;
        if (y_lo < 0) y_lo = 0;

        int xl_lo = std::max(0, leftx_cur  - MARGIN);
        int xl_hi = std::min(w, leftx_cur  + MARGIN);
        int xr_lo = std::max(0, rightx_cur - MARGIN);
        int xr_hi = std::min(w, rightx_cur + MARGIN);

        int sumL = 0, cntL = 0, sumR = 0, cntR = 0;
        for (int y = y_lo; y < y_hi; ++y) {
            const uint8_t* row = bw.ptr<uint8_t>(y);
            for (int x = xl_lo; x < xl_hi; ++x) if (row[x]) {
                lx.push_back(x); ly.push_back(y); sumL += x; cntL++;
            }
            for (int x = xr_lo; x < xr_hi; ++x) if (row[x]) {
                rx.push_back(x); ry.push_back(y); sumR += x; cntR++;
            }
        }
        if (cntL > MIN_PIX) leftx_cur  = sumL / cntL;
        if (cntR > MIN_PIX) rightx_cur = sumR / cntR;
    }

    float lb, lc, rb, rc;
    bool ok_l = linefit(lx, ly, lb, lc);
    bool ok_r = linefit(rx, ry, rb, rc);

    r.ok_right = ok_r;
    r.ok_left  = ok_l;
    r.car_x    = w / 2;
    r.img_h    = h;
    r.img_w    = w;
    if (ok_r) { r.rb = rb; r.rc = rc; }
    if (ok_l) { r.lb = lb; r.lc = lc; }

    if (!ok_r && !ok_l) return r;

    float y_bot = (float)(h - 1);

    if (ok_r) {
        r.heading_err_rad = line_heading(rb);
        r.src = LaneSource::RIGHT;

        if (ok_l) {
            float cx = 0.5f * (lineval(lb, lc, y_bot) + lineval(rb, rc, y_bot));
            r.lateral_offset_m = (cx - (w * 0.5f)) * PIXEL_TO_M;
            r.lane_x = (int)cx;
        } else {
            float rx_bot = lineval(rb, rc, y_bot);
            r.lateral_offset_m = (rx_bot - (w * 0.5f)) * PIXEL_TO_M;
            r.lane_x = (int)rx_bot;
        }
    } else {
        r.heading_err_rad  = line_heading(lb);
        r.lateral_offset_m = (lineval(lb, lc, y_bot) - (w * 0.5f)) * PIXEL_TO_M;
        r.lane_x = (int)lineval(lb, lc, y_bot);
        r.src = LaneSource::LEFT;
    }

    r.ok = true;
    return r;
}

} // namespace lane
