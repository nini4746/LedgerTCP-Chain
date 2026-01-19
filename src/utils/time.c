#include "utils.h"

uint64_t time_current(void) {
    return (uint64_t)time(NULL);
}

uint64_t time_elapsed(uint64_t start) {
    uint64_t now = time_current();
    return now > start ? now - start : 0;
}

uint64_t time_diff(uint64_t start, uint64_t end) {
    return end > start ? end - start : 0;
}

const char *time_to_string(uint64_t time_val) {
    static char buffer[64];
    time_t t = (time_t)time_val;
    struct tm *tm_info = localtime(&t);

    if (tm_info) {
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    } else {
        snprintf(buffer, sizeof(buffer), "%lu", time_val);
    }
    return buffer;
}

void time_sleep(uint64_t ms) {
    (void)ms;
}
