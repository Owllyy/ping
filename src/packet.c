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

packet set_packet(int id, int seq) {
    // ICMP HEADER CONFIGURATION
    struct icmphdr header = (struct icmphdr){
        .cksum = 0,
        .code = 0,          //Error code impossible for type 8
        .id = htons(id),
        .seq = seq,
        .type = ICMP_ECHO,  //Echo request type 8
    };

    packet packet;
    memset(&packet, 0, sizeof(packet));
    packet.header_icmp = header;
    packet.header_icmp.cksum = check_sum(&packet, sizeof(struct packet)); // Checksum
    return packet;
}

void display_response(packet_response * response, float time) {
    char buffer[1024];
    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
    response->header_ip.ip_len, 
    inet_ntop(AF_INET, &response->header_ip.ip_src, buffer, 1024), 
    response->header_icmp.icmp_hun.ih_idseq.icd_seq, 
    response->header_ip.ip_ttl,
    time);
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
    int true = 1;
    int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    setsockopt(socket_fd, SOL_SOCKET, IP_HDRINCL, &true, 0);
    if (socket_fd < 0)
        exit(-1);
    return socket_fd;
}