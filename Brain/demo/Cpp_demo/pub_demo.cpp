#include "ipc/bus.hpp"
#include "messages/imu_state.h"
#include <cstdio>
#include <ctime>
#include <unistd.h>

int main() {
    ipc::Publisher<ImuState> pub(IMU_STATE);
    if (!pub.valid()) { perror("Publisher open"); return 1; }

    uint32_t seq = 0;
    printf("publishing IMU_STATE via libipc C++ Publisher...\n");
    while (true) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ImuState msg = {
            .h     = { .topic = IMU_STATE, .seq = seq, .ts_ns = (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec },
            .roll  = 0.01f * (float)seq,
            .pitch = 0.02f * (float)seq,
            .yaw   = 0.03f * (float)seq,
            .ax = 0.0f, .ay = 0.0f, .az = 9.81f,
        };
        pub.send(msg);
        printf("pub seq=%u roll=%.2f pitch=%.2f yaw=%.2f\n", seq, msg.roll, msg.pitch, msg.yaw);
        seq++;
        sleep(1);
    }
}
