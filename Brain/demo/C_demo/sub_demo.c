#include "ipc/bus.h"
#include "messages/imu_state.h"
#include <stdio.h>

int main(void) {
    ipc_subscriber_t* sub = ipc_subscribe_open(IMU_STATE);
    if (!sub) { perror("ipc_subscribe_open"); return 1; }

    printf("subscribing IMU_STATE via libipc...\n");
    while (1) {
        ImuState msg;
        int r = ipc_poll(sub, &msg, sizeof(msg), 2000);
        if (r == 0)
            printf("sub seq=%u roll=%.2f pitch=%.2f yaw=%.2f az=%.2f\n",
                   msg.h.seq, msg.roll, msg.pitch, msg.yaw, msg.az);
        else if (r == 1)
            printf("timeout\n");
        else
            fprintf(stderr, "poll error\n");
    }
    ipc_subscribe_close(sub);
}
