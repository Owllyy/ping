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

static int get_packet_id(struct ip *ip_hdr, struct icmp *icmp_hdr) {
    if (icmp_hdr->icmp_type == ICMP_ECHOREPLY) {
        return ntohs(icmp_hdr->icmp_hun.ih_idseq.icd_id);
    } else if (icmp_hdr->icmp_type == ICMP_DEST_UNREACH || icmp_hdr->icmp_type == ICMP_TIME_EXCEEDED) {
        struct ip *inner_ip = (struct ip *)((char *)icmp_hdr + 8);
        struct icmp *inner_icmp = (struct icmp *)((char *)inner_ip + (inner_ip->ip_hl << 2));
        (void)ip_hdr;
        return ntohs(inner_icmp->icmp_hun.ih_idseq.icd_id);
    }
    return -1;
}

static void update_recv_timeout(int socket_fd, struct timeval *deadline) {
    struct timeval now, remaining;
    gettimeofday(&now, 0);
    remaining.tv_sec = deadline->tv_sec - now.tv_sec;
    remaining.tv_usec = deadline->tv_usec - now.tv_usec;
    if (remaining.tv_usec < 0) {
        remaining.tv_sec--;
        remaining.tv_usec += 1000000;
    }
    if (remaining.tv_sec < 0) {
        remaining.tv_sec = 0;
        remaining.tv_usec = 0;
    }
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &remaining, sizeof(remaining));
}

static void ping_loop(int socket_fd, t_args *args, char *src, struct sockaddr_in *dst, statistics *stat) {
    char buffer[1024];
    int seq = 0;
    int pid = getpid();
    struct timeval loop_start, loop_end;
    (void)src;

    memset(buffer, 0, 1024);

    while (keep_running) {
        gettimeofday(&loop_start, 0);

        packet ping = set_packet(pid, seq);
        send_ping(socket_fd, &ping, dst);

        struct timeval deadline;
        deadline.tv_sec = loop_start.tv_sec + 1;
        deadline.tv_usec = loop_start.tv_usec;

        int handled = 0;
        while (!handled) {
            update_recv_timeout(socket_fd, &deadline);

            int received_bytes;
            if (receive_ping(socket_fd, buffer, &received_bytes) < 0) {
                update_stat(stat, 0, 0);
                break;
            }

            struct timeval recv_time;
            gettimeofday(&recv_time, 0);

            struct ip *ip_hdr = (struct ip *)buffer;
            struct icmp *icmp_hdr = (struct icmp *)(buffer + (ip_hdr->ip_hl << 2));
            int received_id = get_packet_id(ip_hdr, icmp_hdr);
            if (received_id != pid) {
                continue;
            }

            handled = 1;
            if (icmp_hdr->icmp_type == ICMP_ECHOREPLY) {
                float rtt = timeval_to_float(recv_time) - timeval_to_float(loop_start);
                display_response(ip_hdr, icmp_hdr, rtt);
                update_stat(stat, 1, rtt);
            } else if (icmp_hdr->icmp_type == ICMP_DEST_UNREACH || icmp_hdr->icmp_type == ICMP_TIME_EXCEEDED) {
                if (args->verbose) {
                    display_error_verbose(ip_hdr, icmp_hdr, received_bytes);
                } else {
                    display_error(ip_hdr, icmp_hdr, received_bytes);
                }
                update_stat(stat, 0, 0);
            }
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