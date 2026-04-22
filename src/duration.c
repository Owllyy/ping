#include <duration.h>

static long seconds_as_microseconds(time_t seconds) {
    return seconds * 1000000;
}

double timeval_to_float(struct timeval time) {
    double ret = ((seconds_as_microseconds(time.tv_sec) + time.tv_usec) / 1000.0);
    return ret;
}

double duration_in_ms(struct timeval from) {
    struct timeval to;
    gettimeofday(&to, 0);

    return timeval_to_float(to) - timeval_to_float(from);
}

void sleep_remaining_loop_duration(struct timeval loop_start) {
    struct timeval now;
    gettimeofday(&now, 0);

    long elapsed_us = seconds_as_microseconds(now.tv_sec - loop_start.tv_sec) + (now.tv_usec - loop_start.tv_usec);
    long sleep_time = LOOP_DURATION_IN_MICRO_SECONDS - elapsed_us;

    if (sleep_time > 0) {
        usleep(sleep_time);
    }
}

struct timeval init_loop_deadline(struct timeval loop_start) {
    struct timeval deadline;
    deadline.tv_sec = loop_start.tv_sec + LOOP_DURATION_IN_SECONDS;
    deadline.tv_usec = loop_start.tv_usec;
    return deadline;
}

// To ensure recv timeout consistency if recv is receiving a non handled packet
void update_loop_deadline(int socket_fd, struct timeval *deadline) {
    struct timeval now, remaining;
    gettimeofday(&now, 0);
    remaining.tv_sec = deadline->tv_sec - now.tv_sec;
    remaining.tv_usec = deadline->tv_usec - now.tv_usec;
    if (remaining.tv_usec < 0) {
        remaining.tv_sec--;
        remaining.tv_usec += 1000000;
    }
    if (remaining.tv_sec < 0 || (remaining.tv_sec == 0 && remaining.tv_usec <= 0)) {
        remaining.tv_sec = 0;
        remaining.tv_usec = 1;
    }
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &remaining, sizeof(remaining));
}