#include "ipc/bus.hpp"
#include "messages/camera_frame.h"
#include <cstdio>
#include <ctime>
#include <unistd.h>

int main() {
    ipc::Publisher<CameraFrame> pub(CAMERA_FRAME, sizeof(CameraFrame));
    if (!pub.valid()) { perror("Publisher open"); return 1; }

    uint32_t seq = 0;
    printf("publishing CAMERA_FRAME via libipc C++ Publisher (SHM)...\n");
    while (true) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        CameraFrame msg = {
            .h         = { .topic = CAMERA_FRAME, .seq = seq, .ts_ns = (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec },
            .width     = 640,
            .height    = 480,
            .stride    = 640 * 3,
            .fmt       = 1,
            .data_size = 640 * 3 * 480,
        };
        pub.send(msg);
        printf("pub seq=%u %ux%u fmt=%u\n", seq, msg.width, msg.height, msg.fmt);
        seq++;
        sleep(1);
    }
}
