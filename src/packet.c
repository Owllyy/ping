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
    PACKET_SIZE,
    inet_ntop(AF_INET, &ip_hdr->ip_src, buffer, 1024),
    ntohs(icmp_hdr->icmp_hun.ih_idseq.icd_seq),
    ip_hdr->ip_ttl,
    time);
}

static const char *get_error_message(struct icmp *icmp_hdr) {
    if (icmp_hdr->icmp_type == ICMP_DEST_UNREACH) {
        if (icmp_hdr->icmp_code == ICMP_HOST_UNREACH)
            return "Destination Host Unreachable";
        else if (icmp_hdr->icmp_code == ICMP_NET_UNREACH)
            return "Destination Network Unreachable";
        else if (icmp_hdr->icmp_code == ICMP_PORT_UNREACH)
            return "Destination Port Unreachable";
        else if (icmp_hdr->icmp_code == ICMP_PROT_UNREACH)
            return "Destination Protocol Unreachable";
        else
            return "Destination Unreachable";
    } else if (icmp_hdr->icmp_type == ICMP_TIME_EXCEEDED) {
        return "Time to live exceeded";
    }
    return "Unknown ICMP";
}

void display_error(struct ip *ip_hdr, struct icmp *icmp_hdr, int received_bytes) {
    char src_buffer[INET_ADDRSTRLEN];
    char hostname[1024] = {0};
    struct sockaddr_in sa;

    inet_ntop(AF_INET, &ip_hdr->ip_src, src_buffer, INET_ADDRSTRLEN);

    sa.sin_family = AF_INET;
    sa.sin_addr = ip_hdr->ip_src;
    getnameinfo((struct sockaddr *)&sa, sizeof(sa), hostname, sizeof(hostname), NULL, 0, 0);

    const char *error_msg = get_error_message(icmp_hdr);
    int icmp_size = received_bytes - (ip_hdr->ip_hl << 2);
    printf("%d bytes from %s (%s): %s\n", icmp_size, hostname, src_buffer, error_msg);
}

void display_error_verbose(struct ip *ip_hdr, struct icmp *icmp_hdr, int received_bytes) {
    char src_buffer[INET_ADDRSTRLEN];
    char dst_buffer[INET_ADDRSTRLEN];
    char hostname[1024] = {0};
    struct sockaddr_in sa;

    inet_ntop(AF_INET, &ip_hdr->ip_src, src_buffer, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &ip_hdr->ip_dst, dst_buffer, INET_ADDRSTRLEN);

    sa.sin_family = AF_INET;
    sa.sin_addr = ip_hdr->ip_src;
    getnameinfo((struct sockaddr *)&sa, sizeof(sa), hostname, sizeof(hostname), NULL, 0, 0);

    const char *error_msg = get_error_message(icmp_hdr);
    int icmp_size = received_bytes - (ip_hdr->ip_hl << 2);
    printf("%d bytes from %s (%s): %s\n", icmp_size, hostname, src_buffer, error_msg);

    printf("IP Hdr Dump:\n ");
    unsigned char *ip_bytes = (unsigned char *)ip_hdr;
    int ip_len = ip_hdr->ip_hl * 4;
    for (int i = 0; i < ip_len; i++) {
        printf("%02x", ip_bytes[i]);
        if (i % 2 == 1)
            printf(" ");
    }
    printf("\n");

    printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n");
    printf(" %x  %x  %02x %04x %04x   %x %04x  %02x  %02x %04x %s  %s \n",
        ip_hdr->ip_v,
        ip_hdr->ip_hl,
        ip_hdr->ip_tos,
        ntohs(ip_hdr->ip_len),
        ntohs(ip_hdr->ip_id),
        (ntohs(ip_hdr->ip_off) & 0xe000) >> 13,
        ntohs(ip_hdr->ip_off) & 0x1fff,
        ip_hdr->ip_ttl,
        ip_hdr->ip_p,
        ntohs(ip_hdr->ip_sum),
        src_buffer,
        dst_buffer);

    struct ip *inner_ip = (struct ip *)((char *)icmp_hdr + 8);
    struct icmp *inner_icmp = (struct icmp *)((char *)inner_ip + (inner_ip->ip_hl << 2));
    printf("ICMP: type %d, code %d, size %d, id 0x%04x, seq 0x%04x\n",
        inner_icmp->icmp_type,
        inner_icmp->icmp_code,
        PACKET_SIZE,
        ntohs(inner_icmp->icmp_hun.ih_idseq.icd_id),
        ntohs(inner_icmp->icmp_hun.ih_idseq.icd_seq));
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