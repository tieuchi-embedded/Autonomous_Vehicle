// camera_sim_node — reads MP4, resizes to 320x240 BGR, publishes CAMERA_FRAME (SHM).
// Loops video on EOF. Sleeps to match source FPS.
//
// Usage: camera_sim_node <video_path> [fps_override]

#include "ipc/bus.h"
#include "messages/camera_frame.h"

#include <opencv2/opencv.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>

static constexpr int OUT_W = 320;
static constexpr int OUT_H = 240;
static constexpr int CHANNELS = 3;  // BGR

static std::atomic<bool> g_run{true};
static void on_signal(int) { g_run.store(false); }

int main(int argc, char* argv[]) {
    std::signal(SIGINT,  on_signal);
    std::signal(SIGTERM, on_signal);

    if (argc < 2) {
        std::fprintf(stderr, "usage: %s <video_path> [fps_override]\n", argv[0]);
        return 1;
    }
    const char* video_path = argv[1];

    cv::VideoCapture cap(video_path);
    if (!cap.isOpened()) {
        std::fprintf(stderr, "camera_sim_node: cannot open %s\n", video_path);
        return 1;
    }

    double src_fps = cap.get(cv::CAP_PROP_FPS);
    if (src_fps <= 0.0) src_fps = 30.0;
    double fps = (argc >= 3) ? std::atof(argv[2]) : src_fps;
    auto period = std::chrono::microseconds(static_cast<long>(1.0e6 / fps));

    const size_t pixel_bytes = (size_t)OUT_W * OUT_H * CHANNELS;
    const size_t payload     = sizeof(CameraFrame) + pixel_bytes;

    ipc_publisher_t* pub = ipc_publish_open(CAMERA_FRAME, payload);
    if (!pub) {
        std::fprintf(stderr, "camera_sim_node: ipc_publish_open failed\n");
        return 1;
    }

    std::printf("camera_sim_node: %s @ %.1f fps -> %dx%d BGR\n",
                video_path, fps, OUT_W, OUT_H);

    std::vector<uint8_t> buf(payload);
    CameraFrame* hdr = reinterpret_cast<CameraFrame*>(buf.data());
    uint8_t*     px  = buf.data() + sizeof(CameraFrame);

    cv::Mat frame, resized;
    uint32_t seq = 0;
    auto next_tick = std::chrono::steady_clock::now();

    while (g_run.load()) {
        if (!cap.read(frame) || frame.empty()) {
            // loop video
            cap.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }

        cv::resize(frame, resized, {OUT_W, OUT_H});
        if (!resized.isContinuous()) resized = resized.clone();

        std::memset(hdr, 0, sizeof(*hdr));
        hdr->h.topic   = CAMERA_FRAME;
        hdr->h.seq     = seq;
        hdr->width     = OUT_W;
        hdr->height    = OUT_H;
        hdr->stride    = OUT_W * CHANNELS;
        hdr->fmt       = 0;  // BGR
        hdr->data_size = (uint32_t)pixel_bytes;
        std::memcpy(px, resized.data, pixel_bytes);

        ipc_publish(pub, buf.data(), payload);
        seq++;

        next_tick += period;
        std::this_thread::sleep_until(next_tick);
    }

    ipc_publish_close(pub);
    std::printf("camera_sim_node: shutting down (seq=%u)\n", seq);
    return 0;
}
