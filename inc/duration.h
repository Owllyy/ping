#pragma once
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>

#define LOOP_DURATION_IN_MICRO_SECONDS 1000000
#define LOOP_DURATION_IN_SECONDS 1

double timeval_to_float(struct timeval time);
void sleep_remaining_loop_duration(struct timeval loop_start);
void update_loop_deadline(int socket_fd, struct timeval *deadline);
struct timeval init_loop_deadline(struct timeval loop_start);