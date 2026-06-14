#include "serial/protocol.h"

#include <stdio.h>
#include <string.h>

// Strip a trailing ";;" terminator in place by null-terminating before it.
// Returns the (possibly shortened) string for convenience.
static const char* strip_term(const char* body, char* buf, size_t buflen) {
    size_t n = strlen(body);
    if (n >= 2 && body[n - 2] == ';' && body[n - 1] == ';')
        n -= 2;
    if (n >= buflen) n = buflen - 1;
    memcpy(buf, body, n);
    buf[n] = '\0';
    return buf;
}

int parse_imu(const char* line, float* yaw, float* pitch, float* roll) {
    // Expected: @IMU:yaw,pitch,roll;;
    if (strncmp(line, "@IMU:", 5) != 0) return 0;

    char body[64];
    strip_term(line + 5, body, sizeof(body));

    return sscanf(body, "%f,%f,%f", yaw, pitch, roll) == 3;
}

int parse_rpm(const char* line, float* rpm) {
    // Expected: @RPM:rpm;;
    if (strncmp(line, "@RPM:", 5) != 0) return 0;

    char body[64];
    strip_term(line + 5, body, sizeof(body));

    return sscanf(body, "%f", rpm) == 1;
}
