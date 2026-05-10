#include "ipc/bus.hpp"
#include "messages/ego_state.h"
#include <cstdio>

int main() {
    ipc::Subscriber<EgoState> sub(EGO_STATE);
    if (!sub.valid()) { perror("Subscriber open"); return 1; }

    printf("subscribing EGO_STATE via libipc C++ Subscriber...\n");
    while (true) {
        auto msg = sub.poll(2000);
        if (msg)
            printf("sub seq=%u time_ms=%u angle=%.2f speed=%.2f\n",
                   msg->h.seq, msg->time_ms, msg->angle, msg->speed);
        else
            printf("timeout\n");
    }
}
