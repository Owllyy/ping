#pragma once

typedef struct s_args {
    int verbose;
    int custom_ttl;
    char *address;
} t_args;

void parse_arguments(int ac, char **av, t_args *args);
