#include "ipc/bus.hpp"
#include "messages/imu_state.h"
#include <cstdio>

int main() {
    ipc::Subscriber<ImuState> sub(IMU_STATE);
    if (!sub.valid()) { perror("Subscriber open"); return 1; }

    printf("subscribing IMU_STATE via libipc C++ Subscriber...\n");
    while (true) {
        auto msg = sub.poll(2000);
        if (msg)
            printf("sub seq=%u roll=%.2f pitch=%.2f yaw=%.2f az=%.2f\n",
                   msg->h.seq, msg->roll, msg->pitch, msg->yaw, msg->az);
        else
            printf("timeout\n");
    }
}
