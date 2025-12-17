#pragma once

// An implementation of the naive algorythm found at : https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
// The objective was to calculate variance and mean incrementaly : without keeping a vector of value constantly growing

struct naive_variable {
    float K;
    float Ex;
    float Ex2;
    int n;
};

void add_variable(float x, struct naive_variable *var);
float get_mean(struct naive_variable var);
float get_variance(struct naive_variable var);
float ft_sqrt(float n);