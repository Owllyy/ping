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
    float loss = 100.0;
    if (stat.transmitted > 0) {
        loss = (1.0 - (float)stat.received / (float)stat.transmitted) * 100.0;
    }
    printf("%d packets transmitted, %d packets received, %.1f%% packet loss\n", stat.transmitted, stat.received, loss);
    if (stat.received) {
        printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
        stat.min,
        get_mean(stat.var),
        stat.max,
        get_variance(stat.var));
    }
}

void display_final_stats(char *address, statistics stat) {
    printf("\n--- %s ping statistics ---\n", address);
    display_statistics(stat);
}