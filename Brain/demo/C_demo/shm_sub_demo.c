#include "ipc/bus.h"
#include "messages/camera_frame.h"
#include <stdio.h>

int main(void) {
    ipc_subscriber_t* sub = ipc_subscribe_open(CAMERA_FRAME);
    if (!sub) { perror("ipc_subscribe_open"); return 1; }

    printf("subscribing CAMERA_FRAME via libipc SHM...\n");
    while (1) {
        CameraFrame msg;
        int r = ipc_poll(sub, &msg, sizeof(msg), 2000);
        if (r == 0)
            printf("sub seq=%u %ux%u fmt=%u data_size=%u\n",
                   msg.h.seq, msg.width, msg.height, msg.fmt, msg.data_size);
        else if (r == 1)
            printf("timeout\n");
        else
            fprintf(stderr, "poll error\n");
    }
    ipc_subscribe_close(sub);
}
