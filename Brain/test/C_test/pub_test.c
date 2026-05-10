#include "ipc/bus.h"
#include "messages/ego_state.h"
#include <stdio.h>
#include <unistd.h>

int main(void) {
    ipc_publisher_t* pub = ipc_publish_open(EGO_STATE, sizeof(EgoState));
    if (!pub) { perror("ipc_publish_open"); return 1; }

    uint32_t seq = 0;
    printf("publishing EGO_STATE via libipc...\n");
    while (1) {
        EgoState msg = {
            .h       = { .topic = EGO_STATE, .seq = seq },
            .time_ms = seq * 1000,
            .angle   = 0.5f * (float)seq,
            .speed   = 10.0f,
        };
        ipc_publish(pub, &msg, sizeof(msg));
        printf("pub seq=%u time_ms=%u angle=%.2f speed=%.2f\n",
               seq, msg.time_ms, msg.angle, msg.speed);
        seq++;
        sleep(1);
    }
    ipc_publish_close(pub);
}
