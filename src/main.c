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
    struct timeval loop_start, loop_end;
    (void)src;

    memset(buffer, 0, 1024);

    while (keep_running) {
        gettimeofday(&loop_start, 0);

        packet ping = set_packet(getpid(), seq);
        float rtt;
        int received_bytes;
        int result = send_and_receive_ping(socket_fd, &ping, dst, buffer, &rtt, &received_bytes);

        if (result == 0) {
            struct ip *ip_hdr = (struct ip *)buffer;
            struct icmp *icmp_hdr = (struct icmp *)(buffer + (ip_hdr->ip_hl << 2));

            if (icmp_hdr->icmp_type == ICMP_ECHOREPLY) {
                int received_id = ntohs(icmp_hdr->icmp_hun.ih_idseq.icd_id);
                if (received_id == getpid()) {
                    display_response(ip_hdr, icmp_hdr, rtt);
                    update_stat(stat, 1, rtt);
                }
            } else if (icmp_hdr->icmp_type == ICMP_DEST_UNREACH || icmp_hdr->icmp_type == ICMP_TIME_EXCEEDED) {
                struct ip *inner_ip = (struct ip *)((char *)icmp_hdr + 8);
                struct icmp *inner_icmp = (struct icmp *)((char *)inner_ip + (inner_ip->ip_hl << 2));
                int received_id = ntohs(inner_icmp->icmp_hun.ih_idseq.icd_id);
                if (received_id == getpid()) {
                    if (args->verbose) {
                        display_error_verbose(ip_hdr, icmp_hdr, received_bytes);
                    } else {
                        display_error(ip_hdr, icmp_hdr, received_bytes);
                    }
                    update_stat(stat, 0, 0);
                }
            }
        } else {
            update_stat(stat, 0, 0);
        }

        gettimeofday(&loop_end, 0);
        long elapsed_us = (loop_end.tv_sec - loop_start.tv_sec) * 1000000 + (loop_end.tv_usec - loop_start.tv_usec);
        long sleep_time = 1000000 - elapsed_us;

        if (sleep_time > 0) {
            usleep(sleep_time);
        }

        seq++;
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