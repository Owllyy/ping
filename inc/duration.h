#pragma once
#include <sys/time.h>

double timeval_to_float(struct timeval time);
double duration_in_ms(struct timeval from);
int is_timeout(struct timeval start, unsigned int timeout_ms);