#include <main.h>
#include <args.h>
#include <network.h>
#include <packet.h>

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

static void ping_loop(int socket_fd, t_args *args, char *src, struct sockaddr_in *dst, statistics *stat) {
    char buffer[1024];
    int seq = 0;
    (void)src;
    (void)args;

    memset(buffer, 0, 1024);

    while (keep_running) {
        packet ping = set_packet(getpid(), seq);
        float rtt;
        int result = send_and_receive_ping(socket_fd, &ping, dst, buffer, &rtt);

        if (result == 0) {
            struct ip *ip_hdr = (struct ip *)buffer;
            struct icmp *icmp_hdr = (struct icmp *)(buffer + (ip_hdr->ip_hl << 2));

            display_response(ip_hdr, icmp_hdr, rtt);
            update_stat(stat, 1, rtt);
        } else {
            update_stat(stat, 0, 0);
        }

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
    setup_network(&args, &socket_fd, src, &dst, getpid());
    setup_signal_handler(&stat, args.address);
    ping_loop(socket_fd, &args, src, dst, &stat);
    display_final_stats(args.address, stat);
}