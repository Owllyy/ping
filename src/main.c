#include <main.h>
#include <args.h>
#include <network.h>

static statistics *g_stat = NULL;
static char *g_address = NULL;
static volatile sig_atomic_t keep_running = 1;

void signal_handler(int signum) {
    (void)signum;
    keep_running = 0;
}

static void setup_signal_handler(statistics *stat, char *address) {
    g_stat = stat;
    g_address = address;
    signal(SIGINT, signal_handler);
}

static void display_verbose_info(packet *ping, packet *pong) {
    printf("  Request TTL=%d, Reply TTL=%d\n",
           ping->header_ip.ip_ttl,
           pong->header_ip.ip_ttl);
}

static void ping_loop(int socket_fd, t_args *args, char *src, struct sockaddr_in *dst, statistics *stat) {
    char buffer[1024];
    int seq = 0;

    memset(buffer, 0, 1024);

    while (keep_running) {
        packet ping = set_packet_with_ttl(35, seq, src, inet_ntoa(dst->sin_addr), args->custom_ttl);
        float rtt = send_and_receive_ping(socket_fd, &ping, dst, buffer);
        packet *pong = (struct packet*)buffer;

        display_response(pong, rtt);

        if (args->verbose) {
            display_verbose_info(&ping, pong);
        }

        update_stat(stat, 1, rtt);
        seq++;
        usleep(1000000);
    }
}

int main(int ac, char** av) {
    t_args args;
    int socket_fd;
    char src[1024] = {0};
    struct sockaddr_in *dst;
    statistics stat = {0};

    parse_arguments(ac, av, &args);
    setup_network(&args, &socket_fd, src, &dst);
    setup_signal_handler(&stat, args.address);
    ping_loop(socket_fd, &args, src, dst, &stat);
    display_final_stats(args.address, stat);
}