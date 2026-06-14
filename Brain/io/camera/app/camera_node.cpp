// camera_node — captures from a USB camera (V4L2), resizes to 320x180 BGR,
// publishes CAMERA_FRAME (SHM). The camera stream sets the frame rate.
//
// Usage: camera_node [device_index | /dev/videoN]   (default 0)

#include "ipc/ipc.h"
#include "messages/camera_frame.h"

#include <opencv2/opencv.hpp>

#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static constexpr int OUT_W = 320;
static constexpr int OUT_H = 180;
static constexpr int CHANNELS = 3;  // BGR

static std::atomic<bool> g_run{true};
static void on_signal(int) { g_run.store(false); }

int main(int argc, char* argv[]) {
    std::signal(SIGINT,  on_signal);
    std::signal(SIGTERM, on_signal);

    // Open camera: integer index ("0") or device path ("/dev/video0").
    const char* dev = (argc > 1) ? argv[1] : "0";
    cv::VideoCapture cap;
    char* end = nullptr;
    long idx = std::strtol(dev, &end, 10);
    if (*end == '\0') cap.open((int)idx, cv::CAP_V4L2);
    else              cap.open(dev, cv::CAP_V4L2);

    if (!cap.isOpened()) {
        std::fprintf(stderr, "camera_node: cannot open camera %s\n", dev);
        return 1;
    }

    // Request capture geometry; resize anyway to guarantee the output size.
    cap.set(cv::CAP_PROP_FRAME_WIDTH,  OUT_W);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, OUT_H);

    const size_t pixel_bytes = (size_t)OUT_W * OUT_H * CHANNELS;
    const size_t payload     = sizeof(CameraFrame) + pixel_bytes;

    ipc_publisher_t* pub = ipc_publish(CAMERA_FRAME, payload);
    if (!pub) {
        std::fprintf(stderr, "camera_node: ipc_publish failed\n");
        return 1;
    }

    std::printf("camera_node: %s -> %dx%d BGR\n", dev, OUT_W, OUT_H);

    std::vector<uint8_t> buf(payload);
    CameraFrame* hdr = reinterpret_cast<CameraFrame*>(buf.data());
    uint8_t*     px  = buf.data() + sizeof(CameraFrame);

    cv::Mat frame, resized;
    uint32_t seq = 0;

    while (g_run.load()) {
        if (!cap.read(frame) || frame.empty()) continue;  // dropped frame

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

        ipc_push(pub, buf.data(), payload);
        seq++;
    }

    ipc_unpublish(pub);
    std::printf("camera_node: shutting down (seq=%u)\n", seq);
    return 0;
}
