#pragma once
#include "variance.h"
#include <stdio.h>

typedef struct statistics {
    unsigned int transmitted;
    unsigned int received;
    float min;
    float max;
    struct naive_variable var;
} statistics;

void update_stat(statistics * stat, int is_received, float diff);
void display_statistics(statistics stat);
void display_final_stats(char *address, statistics stat);