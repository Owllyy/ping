#include "main.h"

// from rfc 1071
uint16_t check_sum(const void* data, size_t len) {
    uint32_t sum = 0;
    const uint16_t* words = (const uint16_t*)data;
    for (; len > 1; len -= 2)
        sum += *words++;

    if (len != 0)
        sum += *(const uint8_t*)words;
    sum = (sum & 0xffff) + (sum >> 16);
    sum = (sum & 0xffff) + (sum >> 16);
    return ~sum;
}

struct sockaddr_in set_sockaddr_in(char *addr) {
    struct sockaddr_in ret;
    ret.sin_family = AF_INET;
    inet_pton(AF_INET, addr, &ret.sin_addr);
    return ret;
}

packet set_packet(int id, int seq) {
    packet packet;
    memset(&packet, 0, sizeof(packet));

    // ICMP HEADER CONFIGURATION
    packet.header_icmp.icmp_type = ICMP_ECHO; //Echo request type 8
    packet.header_icmp.icmp_code = 0; //Error code impossible for type 8
    packet.header_icmp.icmp_hun.ih_idseq.icd_id = htons(55);
    packet.header_icmp.icmp_hun.ih_idseq.icd_seq = seq;
    packet.header_icmp.icmp_cksum = check_sum(&packet, sizeof(struct packet)); // Checksum
    return packet;
}

struct msghdr set_response() {
    struct msghdr response;
    memset(&response, 0, sizeof(response));

    struct iovec * vec = malloc(sizeof(struct iovec));
    *vec = (struct iovec){
        .iov_base = malloc(1024),
        .iov_len = 1024,
    };
    memset(vec->iov_base, 0, 1024);
    response.msg_iov = vec;
    response.msg_iovlen = 1;
    return response;
}

void display_response(packet_response * response, struct timeval tv) {
    char buffer[1024];
    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%ld.%d ms\n",
    response->header_ip.ip_len, 
    inet_ntop(AF_INET, &response->header_ip.ip_src, buffer, 1024), 
    response->header_icmp.icmp_hun.ih_idseq.icd_seq, 
    response->header_ip.ip_ttl,
    tv.tv_sec,
    tv.tv_usec);
}

void display_statistics(statistics stat) {
    float loss = ((float)stat.received / (float)stat.transmitted - 1) * 100;
    printf("%d packets transmitted, %d packets received, %f%% packets loss\n", stat.transmitted, stat.received, loss);
    if (stat.received) {
        // AVERAGE TIME OF ARRIVAL
        // TODO
        struct timeval avg;
        avg.tv_sec = (stat.max.tv_sec + stat.min.tv_sec) / 2;
        avg.tv_usec = (stat.max.tv_usec + stat.min.tv_usec) / 2;

        // STANDARD DEVIANCE
        // TODO

        printf("round-trip min/avg/max/stddev = %ld.%d/%ld.%d/%ld.%d/todo",
        stat.min.tv_sec,
        stat.min.tv_usec,
        avg.tv_sec,
        avg.tv_usec,
        stat.max.tv_sec,
        stat.max.tv_usec);
    }
}

struct timeval time_diff(struct timeval start, struct timeval end) {
    struct timeval res;

    res.tv_sec = end.tv_sec - start.tv_sec;
    res.tv_usec = end.tv_usec - start.tv_usec;
    return res;
}

int time_comp(struct timeval first, struct timeval last) {
    return first.tv_sec >= last.tv_sec && first.tv_usec > last.tv_usec;
}

void update_stat(statistics * stat, int is_received, struct timeval diff) {
    stat->transmitted++;
    if (is_received) {
        if (stat->received == 0) {
            stat->min = diff;
            stat->max = diff;
        } else {
            if (time_comp(diff, stat->max))
                stat->max = diff;
            else if (time_comp(stat->min, diff))
                stat->min = diff;
        }
        stat->received++;
    }
}

char address[] = "127.0.0.1";

int main() {
    int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (socket_fd < 0)
        return -1;


    struct sockaddr_in dst = set_sockaddr_in(address);
    printf("PING %s (%s): %lu data bytes\n", address, address, 64 - sizeof(struct icmp));

    // STATISTICS INIT
    struct timeval start;
    statistics stat;
    int is_received = 1;
    int i = 0;

    for (;i < 10; i++) {

        // INIT PACKET
        packet packet = set_packet(55, i);
        // INIT RECV
        struct msghdr buffer = set_response();

        // TIME INIT
        gettimeofday(&start, 0);
        // SEND TO
        sendto(socket_fd, &packet, sizeof(struct packet), 0, (struct sockaddr *)&dst, sizeof(struct sockaddr_in));
        // RCV MSG
        recvmsg(socket_fd, &buffer, 0);
        packet_response * response = (struct packet_r *)buffer.msg_iov->iov_base;
        // TIME COMPARAISON
        struct timeval diff;
        gettimeofday(&diff, 0);
        diff = time_diff(start, diff);

        // DISPLAY
        display_response(response, diff);

        // UPDATE STATISTICS
        update_stat(&stat, 1, diff);
        usleep(300000);
    }
    // DISPLAY STATISTICS
    printf("--- %s ping statistics ---\n", address);
    display_statistics(stat);
}