#include "ipc/bus.h"
#include "messages/camera_frame.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main(void) {
    ipc_publisher_t* pub = ipc_publish_open(CAMERA_FRAME, sizeof(CameraFrame));
    if (!pub) { perror("ipc_publish_open"); return 1; }

    uint32_t seq = 0;
    printf("publishing CAMERA_FRAME via libipc SHM...\n");
    while (1) {
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
        ipc_publish(pub, &msg, sizeof(msg));
        printf("pub seq=%u %ux%u fmt=%u data_size=%u\n", seq, msg.width, msg.height, msg.fmt, msg.data_size);
        seq++;
        sleep(1);
    }
    ipc_publish_close(pub);
}
