#include "ipc/bus.h"
#include "messages/serial_data.h"
#include <stdio.h>

int main(void) {
    ipc_subscriber_t* sub = ipc_subscribe_open(SERIAL);
    if (!sub) { perror("ipc_subscribe_open"); return 1; }

    printf("subscribing SERIAL via libipc...\n");
    while (1) {
        SerialData msg;
        int r = ipc_poll(sub, &msg, sizeof(msg), 2000);
        if (r == 0)
            printf("seq=%u yaw=%.2f pitch=%.2f roll=%.2f rpm=%.2f\n",
                   msg.h.seq, msg.yaw, msg.pitch, msg.roll, msg.rpm);
        else if (r == 1)
            printf("timeout\n");
        else
            fprintf(stderr, "poll error\n");
    }
    ipc_subscribe_close(sub);
}
