// lane_node — sub CAMERA_FRAME (BGR), run lane detection, pub LANE_STATE.
//
// Optional debug visualisation: pass --show to display intermediate stages.

#include "ipc/bus.h"
#include "ipc/bus.hpp"
#include "messages/camera_frame.h"
#include "messages/lane_state.h"
#include "lane/preprocessing.hpp"
#include "lane/lane_detector.hpp"
#include "lane/lane_tracker.hpp"

#include <opencv2/opencv.hpp>

#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <vector>

static std::atomic<bool> g_run{true};
static void on_signal(int) { g_run.store(false); }

int main(int argc, char* argv[]) {
    std::signal(SIGINT,  on_signal);
    std::signal(SIGTERM, on_signal);

    bool show = false;
    for (int i = 1; i < argc; ++i)
        if (std::strcmp(argv[i], "--show") == 0) show = true;

    ipc_subscriber_t* cam_sub = ipc_subscribe_open(CAMERA_FRAME);
    if (!cam_sub) {
        std::fprintf(stderr, "lane_node: subscribe CAMERA_FRAME failed\n");
        return 1;
    }

    ipc::Publisher<LaneState> lane_pub(LANE_STATE);
    if (!lane_pub.valid()) {
        std::fprintf(stderr, "lane_node: publish LANE_STATE failed\n");
        return 1;
    }

    std::printf("lane_node: running%s\n", show ? " (--show)" : "");

    std::vector<uint8_t> buf(sizeof(CameraFrame) + 320 * 240 * 3);
    cv::Mat M;
    int M_w = 0, M_h = 0;
    lane::Tracker tracker;

    while (g_run.load()) {
        int rc = ipc_poll(cam_sub, buf.data(), buf.size(), 500);
        if (rc != 0) continue;

        const CameraFrame* hdr = reinterpret_cast<const CameraFrame*>(buf.data());
        const uint8_t*     px  = buf.data() + sizeof(CameraFrame);

        if (hdr->fmt != 0 /*BGR*/) {
            std::fprintf(stderr, "lane_node: unsupported fmt=%u\n", hdr->fmt);
            continue;
        }

        cv::Mat frame((int)hdr->height, (int)hdr->width, CV_8UC3, (void*)px, hdr->stride);

        if (M.empty() || M_w != (int)hdr->width || M_h != (int)hdr->height) {
            M = lane::ipm_matrix((int)hdr->width, (int)hdr->height);
            M_w = (int)hdr->width;
            M_h = (int)hdr->height;
        }

        cv::Mat warped = lane::ipm(frame, M);
        cv::Mat bw     = lane::binary_mask(warped);
        auto    raw    = lane::detect(bw);
        auto    smoothed = tracker.smooth(raw);

        LaneState msg{};
        msg.heading_err_rad  = smoothed.heading_err_rad;
        msg.lateral_offset_m = smoothed.lateral_offset_m;
        lane_pub.send(msg);

        const char* src_str = smoothed.src == lane::LaneSource::RIGHT ? "R" :
                              smoothed.src == lane::LaneSource::LEFT  ? "L" : "-";
        std::printf("lane: ok=%d src=%s heading=%.4f rad  offset=%.4f m\n",
                    smoothed.ok, src_str,
                    smoothed.heading_err_rad,
                    smoothed.lateral_offset_m);

        if (show) {
            // --- frame window: IPM trapezoid ---
            cv::Mat dbg = frame.clone();
            auto pts = lane::ipm_src_points(M_w, M_h);
            std::vector<cv::Point> poly = {
                {(int)pts[0].x, (int)pts[0].y},
                {(int)pts[1].x, (int)pts[1].y},
                {(int)pts[2].x, (int)pts[2].y},
                {(int)pts[3].x, (int)pts[3].y},
            };
            cv::polylines(dbg, poly, true, {0, 255, 0}, 1);

            // --- warped overlay ---
            cv::Mat warped_dbg = warped.clone();
            if (raw.ok) {
                int H = raw.img_h, W = raw.img_w;
                float y_bot = (float)(H - 1);

                // Choose reference poly (right preferred, fallback left)
                float a_ref = raw.ok_right ? raw.ra : raw.la;
                float b_ref = raw.ok_right ? raw.rb : raw.lb;
                float c_ref = raw.ok_right ? raw.rc : raw.lc;

                // Evaluate reference lane x at bottom for anchor point
                float ref_x_bot = a_ref * y_bot * y_bot + b_ref * y_bot + c_ref;

                // LINE 1: curve bám sát lane tham chiếu (xanh lá)
                std::vector<cv::Point> lane_pts;
                for (int y = 0; y < H; ++y) {
                    float yf = (float)y;
                    int   x  = (int)(a_ref * yf * yf + b_ref * yf + c_ref);
                    if (x >= 0 && x < W)
                        lane_pts.push_back({x, y});
                }
                if (lane_pts.size() > 1)
                    cv::polylines(warped_dbg, lane_pts, false,
                                  raw.src == lane::LaneSource::RIGHT
                                      ? cv::Scalar(0, 255, 0)   // green = right
                                      : cv::Scalar(0, 165, 255), // orange = left fallback
                                  2, cv::LINE_AA);

                // LINE 2: thân xe — đường thẳng đứng giữa ảnh (trắng)
                cv::line(warped_dbg, {W / 2, 0}, {W / 2, H},
                         {255, 255, 255}, 1, cv::LINE_AA);

                // LINE 3: song song LINE 1, dính vào LINE 2 tại bottom
                // Offset = car_x - ref_x_bot (shift ngang để line đi qua tâm xe)
                float shift = (float)(W / 2) - ref_x_bot;
                std::vector<cv::Point> car_pts;
                for (int y = 0; y < H; ++y) {
                    float yf = (float)y;
                    int   x  = (int)(a_ref * yf * yf + b_ref * yf + c_ref + shift);
                    if (x >= 0 && x < W)
                        car_pts.push_back({x, y});
                }
                if (car_pts.size() > 1)
                    cv::polylines(warped_dbg, car_pts, false,
                                  {0, 255, 255}, 2, cv::LINE_AA);  // yellow

                // Offset segment tại bottom nối LINE1 ↔ LINE3 (đỏ)
                cv::line(warped_dbg,
                         {(int)ref_x_bot, H - 1},
                         {W / 2,          H - 1},
                         {0, 0, 255}, 2);

                // HUD text
                const char* src_str = raw.src == lane::LaneSource::RIGHT ? "R" : "L";
                cv::putText(warped_dbg,
                            cv::format("[%s] head=%.3f  off=%.3f m", src_str,
                                       smoothed.heading_err_rad,
                                       smoothed.lateral_offset_m),
                            {4, 16}, cv::FONT_HERSHEY_PLAIN, 0.9,
                            {255, 255, 255}, 1);
            }

            cv::imshow("frame",  dbg);
            cv::imshow("warped", warped_dbg);
            cv::imshow("binary", bw);
            if (cv::waitKey(1) == 27) g_run.store(false);
        }
    }

    ipc_subscribe_close(cam_sub);
    std::printf("lane_node: shutting down\n");
    return 0;
}
