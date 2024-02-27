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

struct sockaddr_in set_sockaddr_in(char *addr) {
    struct sockaddr_in ret;
    ret.sin_family = AF_INET;
    inet_pton(AF_INET, addr, &ret.sin_addr);
    return ret;
}

packet set_packet(int id, int seq, char *src, char *dst) {
    packet packet;
    memset(&packet, 0, sizeof(packet));
    // IP HEADER CONFIGURATION
    packet.header_ip.ip_hl = 5; //header size in int
    packet.header_ip.ip_v = 4; //Ipv4
    packet.header_ip.ip_tos = 0; //Not used
    packet.header_ip.ip_len = sizeof(struct packet); //Total size of packet
    packet.header_ip.ip_id = htons(55); //id for fragmentation
    packet.header_ip.ip_off = 0;
    packet.header_ip.ip_ttl = 62; // Time to live in node
    packet.header_ip.ip_p = IPPROTO_ICMP;
    inet_pton(AF_INET, src, &packet.header_ip.ip_src);
    inet_pton(AF_INET, dst, &packet.header_ip.ip_dst);
    packet.header_ip.ip_sum = check_sum(&packet, sizeof(struct packet));

    // ICMP HEADER CONFIGURATION
    packet.header_icmp.icmp_type = ICMP_ECHO;
    packet.header_icmp.icmp_code = 0;
    packet.header_icmp.icmp_hun.ih_idseq.icd_id = htons(id);
    packet.header_icmp.icmp_hun.ih_idseq.icd_seq = htons(seq);
    packet.header_icmp.icmp_cksum = check_sum(&packet, sizeof(struct packet));

    return packet;
}

void display_response(packet* response, float time) {
    char buffer[1024];
    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
    response->header_ip.ip_len + sizeof(struct ip), 
    inet_ntop(AF_INET, &response->header_ip.ip_src, buffer, 1024), 
    ntohs(response->header_icmp.icmp_hun.ih_idseq.icd_seq), 
    response->header_ip.ip_ttl,
    time);
}

void display_packet(packet* response) {
    char buffer[1024];
    printf("Packet = \n    Ip: hl=%u v=%u tos=%u len=%u id=%u off=%u ttl=%u p=%u sum=%u src=%s dst=%s\n",
    response->header_ip.ip_hl,
    response->header_ip.ip_v,
    response->header_ip.ip_tos,
    response->header_ip.ip_len,
    response->header_ip.ip_id,
    response->header_ip.ip_off,
    response->header_ip.ip_ttl,
    response->header_ip.ip_p,
    response->header_ip.ip_sum,
    inet_ntop(AF_INET, &response->header_ip.ip_src, buffer, 1024),
    inet_ntop(AF_INET, &response->header_ip.ip_dst, buffer, 1024));

    printf("    ICMP: type=%u code=%u cksum=%u id=%u seq=%u\n",
    response->header_icmp.icmp_type,
    response->header_icmp.icmp_code,
    ntohs(response->header_icmp.icmp_cksum),
    ntohs(response->header_icmp.icmp_hun.ih_idseq.icd_id),
    ntohs(response->header_icmp.icmp_hun.ih_idseq.icd_seq));
}


struct sockaddr *dns_resolution(char *fqdn) {
    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;

    
    getaddrinfo(fqdn, 0, &hints, &res);
    return res->ai_addr;
}

int init_socket() {
    int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    int true = 1;
    if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &true, sizeof(true)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    if (socket_fd < 0)
        exit(-1);
    return socket_fd;
}