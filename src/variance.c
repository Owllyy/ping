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

float get_variance(struct naive_variable var) {
    return sqrt(var.Ex2 - var.Ex * var.Ex / var.n) / (var.n - 1);
}