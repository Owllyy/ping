#include <packet.h>

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

packet set_packet(int id, int seq) {
    packet pkt;
    memset(&pkt, 0, sizeof(pkt));

    pkt.header_icmp.icmp_type = ICMP_ECHO;
    pkt.header_icmp.icmp_code = 0;
    pkt.header_icmp.icmp_hun.ih_idseq.icd_id = htons(id);
    pkt.header_icmp.icmp_hun.ih_idseq.icd_seq = htons(seq);
    pkt.header_icmp.icmp_cksum = 0;
    pkt.header_icmp.icmp_cksum = check_sum(&pkt, sizeof(pkt));

    return pkt;
}

void display_response(struct ip *ip_hdr, struct icmp *icmp_hdr, float time) {
    char buffer[1024];
    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
    ntohs(ip_hdr->ip_len),
    inet_ntop(AF_INET, &ip_hdr->ip_src, buffer, 1024),
    ntohs(icmp_hdr->icmp_hun.ih_idseq.icd_seq),
    ip_hdr->ip_ttl,
    time);
}

struct sockaddr *dns_resolution(char *fqdn) {
    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;

    int ret = getaddrinfo(fqdn, 0, &hints, &res);
    if (ret != 0) {
        fprintf(stderr, "ft_ping: %s: %s\n", fqdn, gai_strerror(ret));
        exit(1);
    }
    if (!res) {
        fprintf(stderr, "ft_ping: %s: Unknown host\n", fqdn);
        exit(1);
    }
    return res->ai_addr;
}

int init_socket() {
    int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (socket_fd < 0) {
        perror("ft_ping: socket");
        fprintf(stderr, "ft_ping: (Did you run with sudo?)\n");
        exit(1);
    }

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("ft_ping: setsockopt");
        exit(1);
    }

    return socket_fd;
}

void set_socket_ttl(int socket_fd, int ttl) {
    if (setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
        perror("ft_ping: setsockopt IP_TTL");
        exit(1);
    }
}