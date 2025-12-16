#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "args.h"

static void print_usage(void) {
    printf("Usage: ft_ping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf("  -v            verbose output\n");
    printf("  --ttl TTL     set Time To Live (1-255)\n");
    printf("  --help        print help\n");
    printf("  --usage       print usage\n");
}

static void init_args(t_args *args) {
    args->verbose = 0;
    args->custom_ttl = 64;
    args->address = NULL;
}

static int parse_ttl_option(int ac, char **av, int i, t_args *args) {
    if (i + 1 >= ac) {
        fprintf(stderr, "ft_ping: option '--ttl' requires an argument\n");
        exit(1);
    }
    args->custom_ttl = atoi(av[i + 1]);
    if (args->custom_ttl < 1 || args->custom_ttl > 255) {
        fprintf(stderr, "ft_ping: invalid TTL value: %d (must be 1-255)\n", args->custom_ttl);
        exit(1);
    }
    return i + 2;
}

static void handle_help_options(char *option) {
    if (!strcmp(option, "--usage")) {
        printf("Usage: ft_ping [-v?V] [-T NUM] [--ttl=NUM] [--verbose] [--help]\n");
        printf("            [--usage] HOST ...\n");
        exit(0);
    }
    if (!strcmp(option, "--help") || !strcmp(option, "-?")) {
        print_usage();
        exit(0);
    }
}

static void handle_unknown_option(char *option) {
    if (option[0] == '-' && option[1] != '-') {
        fprintf(stderr, "ft_ping: invalid option -- '%c'\n", option[1]);
        fprintf(stderr, "Try 'ft_ping --help' or 'ft_ping --usage' for more information.\n");
        exit(1);
    }
    fprintf(stderr, "ft_ping: unrecognized option '%s'\n", option);
    fprintf(stderr, "Try 'ft_ping --help' or 'ft_ping --usage' for more information.\n");
    exit(1);
}

static int parse_single_option(int ac, char **av, int i, t_args *args) {
    if (!strcmp(av[i], "--ttl")) {
        return parse_ttl_option(ac, av, i, args);
    }
    handle_help_options(av[i]);
    if (!strcmp(av[i], "-v")) {
        args->verbose = 1;
        return i + 1;
    }
    handle_unknown_option(av[i]);
    return i;
}

void parse_arguments(int ac, char **av, t_args *args) {
    int i = 1;
    int destination_found = 0;

    init_args(args);

    while (i < ac) {
        if (av[i][0] == '-') {
            i = parse_single_option(ac, av, i, args);
        } else {
            if (destination_found) {
                fprintf(stderr, "ft_ping: extra operand '%s'\n", av[i]);
                exit(1);
            }
            args->address = av[i];
            destination_found = 1;
            i++;
        }
    }

    if (!destination_found) {
        fprintf(stderr, "ft_ping: missing host operand\n");
        fprintf(stderr, "Try 'ft_ping --help' or 'ft_ping --usage' for more information.\n");
        exit(1);
    }
}
