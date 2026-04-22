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

static void handle_response_display(char *buffer, int received_bytes, struct timeval loop_start, t_args *args, statistics *stat) {
    struct ip *ip_hdr = (struct ip *)buffer;
    struct icmp *icmp_hdr = (struct icmp *)(buffer + (ip_hdr->ip_hl << 2));

    struct timeval recv_time;
    gettimeofday(&recv_time, 0);

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

static bool is_response_valid(char *buffer, int ping_id) {
    struct ip *ip_hdr = (struct ip *)buffer;
    struct icmp *icmp_hdr = (struct icmp *)(buffer + (ip_hdr->ip_hl << 2));

    return get_packet_id(ip_hdr, icmp_hdr) == ping_id;
}

static void ping_loop(int socket_fd, t_args *args, struct sockaddr_in *dst, statistics *stat) {
    char buffer[1024] = {0};
    int received_bytes = 0;
    int seq = 0;
    int ping_id = getpid();
    struct timeval loop_start;

    while (keep_running) {
        gettimeofday(&loop_start, 0);

        packet ping = init_packet(ping_id, seq);
        send_ping(socket_fd, &ping, dst);
        
        struct timeval deadline = init_loop_deadline(loop_start);

        while (1) {
            update_loop_deadline(socket_fd, &deadline);
            
            // Timeout
            if (receive_ping(socket_fd, buffer, &received_bytes) < 0) {
                
                update_stat(stat, 0, 0);
                goto next_ping;
            }

            if(is_response_valid(buffer, ping_id))
                break;
        }

        handle_response_display(buffer, received_bytes, loop_start, args, stat);
        next_ping:
            sleep_remaining_loop_duration(loop_start);
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
    ping_loop(socket_fd, &args, dst, &stat);
    display_final_stats(args.address, stat);
}