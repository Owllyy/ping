#include "main.h"

typedef struct packet {
    struct icmp header_icmp;
    struct ip header_ip;
    char msg[64];
} packet;


struct sockaddr_in set_sockaddr_in(char *addr) {
    struct sockaddr_in ret;
    ret.sin_family = AF_INET;
    inet_pton(AF_INET, addr, &ret.sin_addr);
    return ret;
}

struct msghdr set_response() {
    struct msghdr response;
    memset(&response, 0, sizeof(response));

    struct iovec vec = (struct iovec){
        .iov_base = malloc(1024),
        .iov_len = 1024,
    };
    response.msg_iov = &vec;
    response.msg_iovlen = 1;
    return response;
}

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


int main() {
    int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (socket_fd < 0)
        return -1;

    struct sockaddr_in src = set_sockaddr_in("127.0.0.1");
    struct sockaddr_in dst = set_sockaddr_in("127.0.0.1");

    packet packet;
    memset(&packet, 0, sizeof(packet));

    // IP HEADER CONFIGURATION
    packet.header_ip.ip_v = 4; //Ipv4
    packet.header_ip.ip_hl = 5; //header size in int
    packet.header_ip.ip_tos = 0; //Not used
    packet.header_ip.ip_id = htons(520); //id for fragmentation
    packet.header_ip.ip_off = 0; //todo
    packet.header_ip.ip_ttl = 64; // Time to live in node
    packet.header_ip.ip_sum = 0; //todo
    packet.header_ip.ip_src = src.sin_addr; // SRC ip big_endian
    packet.header_ip.ip_dst = dst.sin_addr; // Destination ip big_endian
    packet.header_ip.ip_len = htons(sizeof(struct packet)); //Total size of packet
    packet.header_ip.ip_p = 0; //protocol

    // ICMP HEADER CONFIGURATION
    packet.header_icmp.icmp_type = ICMP_ECHO; //Echo request type 8
    packet.header_icmp.icmp_code = 0; //Error code impossible for type 8
    packet.header_icmp.icmp_hun.ih_idseq.icd_id = htons(520);
    packet.header_icmp.icmp_hun.ih_idseq.icd_seq = 1;
    packet.header_icmp.icmp_cksum = check_sum(&packet, sizeof(packet)); // Checksum todo



    struct msghdr response;
    // memset(&response, 0, sizeof(response));

    char buffer[1024];
    struct iovec vec = (struct iovec){
        .iov_base = buffer,
        .iov_len = sizeof(buffer),
    };
    response.msg_iov = &vec;
    response.msg_iovlen = 1;

    // SEND TO
    sendto(socket_fd, &packet, sizeof(struct packet), 0, (struct sockaddr *)&dst, sizeof(struct sockaddr_in));
    // RCV MSG
    printf("%zd\n", recvmsg(socket_fd, &response, 0));
}