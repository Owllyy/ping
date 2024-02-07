#include "main.h"

typedef struct packet {
    struct icmp header_icmp;
    char msg[64];
} packet;

typedef struct packet_r {
    struct ip header_ip;
    struct icmp header_icmp;
    char msg[64];
} packet_r;

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


int main() {
    int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (socket_fd < 0)
        return -1;

    struct sockaddr_in src = set_sockaddr_in("127.0.0.1");
    struct sockaddr_in dst = set_sockaddr_in("127.0.0.1");

    packet packet;
    memset(&packet, 0, sizeof(packet));

    // ICMP HEADER CONFIGURATION
    packet.header_icmp.icmp_type = ICMP_ECHO; //Echo request type 8
    packet.header_icmp.icmp_code = 0; //Error code impossible for type 8
    packet.header_icmp.icmp_hun.ih_idseq.icd_id = htons(55);
    packet.header_icmp.icmp_hun.ih_idseq.icd_seq = 1;
    packet.header_icmp.icmp_cksum = check_sum(&packet, sizeof(struct packet)); // Checksum
    printf("Initial checksum : %d\n", packet.header_icmp.icmp_cksum);



    struct msghdr response;
    memset(&response, 0, sizeof(response));

    char name[1024];
    memset(name, 0, 1024);
    response.msg_name = &name;
    response.msg_namelen = 1024;
    char control[1024];
    memset(control, 0, 1024);
    response.msg_control = &control;
    response.msg_controllen = 1024;
    char buffer[1024];
    memset(buffer, 0, 1024);
    struct iovec vec = (struct iovec){
        .iov_base = buffer,
        .iov_len = sizeof(buffer),
    };
    response.msg_iov = &vec;
    response.msg_iovlen = 1;

    // SEND TO
    printf("%d\n", sendto(socket_fd, &packet, sizeof(struct packet), 0, (struct sockaddr *)&dst, sizeof(struct sockaddr_in)));
    // RCV MSG
    printf("%zd\n", recvmsg(socket_fd, &response, 0));

    packet_r * respons = (struct packet_r *)buffer;
    printf("%s > %s: ", inet_ntop(AF_INET, &respons->header_ip.ip_src, name, 1024), inet_ntop(AF_INET, &respons->header_ip.ip_dst, name, 1024));
    printf("type %d, error_code %d, id %d, seq %d, lenght %d\n\n", respons->header_icmp.icmp_type, respons->header_icmp.icmp_code, respons->header_icmp.icmp_hun.ih_idseq.icd_id, respons->header_icmp.icmp_hun.ih_idseq.icd_seq, respons->header_ip.ip_len);
}