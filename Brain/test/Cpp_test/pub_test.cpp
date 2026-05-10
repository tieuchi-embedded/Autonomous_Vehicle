#include "ipc/bus.hpp"
#include "messages/ego_state.h"
#include <cstdio>
#include <unistd.h>

int main() {
    ipc::Publisher<EgoState> pub(EGO_STATE);
    if (!pub.valid()) { perror("Publisher open"); return 1; }

    uint32_t seq = 0;
    printf("publishing EGO_STATE via libipc C++ Publisher...\n");
    while (true) {
        EgoState msg{};
        msg.h.topic = EGO_STATE;
        msg.h.seq   = seq;
        msg.time_ms = seq * 1000;
        msg.angle   = 0.5f * (float)seq;
        msg.speed   = 10.0f;
        pub.send(msg);
        printf("pub seq=%u time_ms=%u angle=%.2f speed=%.2f\n",
               seq, msg.time_ms, msg.angle, msg.speed);
        seq++;
        sleep(1);
    }
}
