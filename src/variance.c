#include <variance.h>

void add_variable(float x, struct naive_variable *var) {
    if (var->n == 0)
        var->K = x;
    var->n += 1;
    var->Ex += x - var->K;
    var->Ex2 += (x - var->K) * (x - var->K);
}

float get_mean(struct naive_variable var) {
    return var.K + var.Ex / var.n;
}

float ft_sqrt(float n) {
    if (n < 0)
        return 0;
    if (n == 0)
        return 0;

    float x = n;
    float prev;
    int i = 0;

    while (i < 10) {
        prev = x;
        x = (x + n / x) / 2;
        if (x == prev)
            break;
        i++;
    }
    return x;
}

float get_variance(struct naive_variable var) {
    return ft_sqrt(var.Ex2 - var.Ex * var.Ex / var.n) / (var.n - 1);
}