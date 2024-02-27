#include <duration.h>

double timeval_to_float(struct timeval time) {
    double ret = (((double)time.tv_sec * 1000000 + (double)time.tv_usec) / 1000.0);\
    return ret;
}

double duration_in_ms(struct timeval from) {
    struct timeval end;
    gettimeofday(&end, 0);

    return timeval_to_float(end) - timeval_to_float(from);
}

int is_timeout(struct timeval start, unsigned int timeout_ms) {
    struct timeval now;

    gettimeofday(&now, 0);
    if (duration_in_ms(start) > timeout_ms)
        return 1;
    return 0;
}