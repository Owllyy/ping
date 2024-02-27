#include <stat.h>

void update_stat(statistics * stat, int is_received, float diff) {
    stat->transmitted++;
    if (is_received) {
        if (stat->received == 0) {
            stat->min = diff;
            stat->max = diff;
        } else {
            if (diff > stat->max)
                stat->max = diff;
            else if (diff < stat->min)
                stat->min = diff;
        }
        add_variable(diff, &stat->var);
        stat->received++;
    }
}

void display_statistics(statistics stat) {
    float loss = ((float)stat.received / (float)stat.transmitted - 1) * 100;
    printf("%d packets transmitted, %d packets received, %.3f%% packets loss\n", stat.transmitted, stat.received, loss);
    if (stat.received) {
        printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
        stat.min,
        get_mean(stat.var),
        stat.max,
        get_variance(stat.var));
    }
}