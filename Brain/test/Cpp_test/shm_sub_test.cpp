#include "ipc/bus.hpp"
#include "messages/camera_frame.h"
#include <cstdio>

int main() {
    ipc::Subscriber<CameraFrame> sub(CAMERA_FRAME);
    if (!sub.valid()) { perror("Subscriber open"); return 1; }

    printf("subscribing CAMERA_FRAME via libipc C++ Subscriber (SHM)...\n");
    while (true) {
        auto msg = sub.poll(2000);
        if (msg)
            printf("sub seq=%u %ux%u fmt=%u data_size=%u\n",
                   msg->h.seq, msg->width, msg->height, msg->fmt, msg->data_size);
        else
            printf("timeout\n");
    }
}
