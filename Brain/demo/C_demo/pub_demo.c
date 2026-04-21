#include "ipc/bus.h"
#include "messages/imu_state.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main(void) {
    ipc_publisher_t* pub = ipc_publish_open(IMU_STATE, sizeof(ImuState));
    if (!pub) { perror("ipc_publish_open"); return 1; }

    uint32_t seq = 0;
    printf("publishing IMU_STATE via libipc...\n");
    while (1) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ImuState msg = {
            .h     = { .topic = IMU_STATE, .seq = seq, .ts_ns = (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec },
            .roll  = 0.01f * (float)seq,
            .pitch = 0.02f * (float)seq,
            .yaw   = 0.03f * (float)seq,
            .ax = 0.0f, .ay = 0.0f, .az = 9.81f,
        };
        ipc_publish(pub, &msg, sizeof(msg));
        printf("pub seq=%u roll=%.2f pitch=%.2f yaw=%.2f\n", seq, msg.roll, msg.pitch, msg.yaw);
        seq++;
        sleep(1);
    }
    ipc_publish_close(pub);
}
