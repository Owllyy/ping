#include <main.h>

void get_src_address(char *buffer) {
   struct ifaddrs *addrs, *tmp;
    getifaddrs(&addrs);
    tmp = addrs;

    while (tmp) {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            printf("Interface: %s\tAddress: %s\n", tmp->ifa_name, inet_ntoa(pAddr->sin_addr));
            if (!strcmp(tmp->ifa_name, "en0")) {
                inet_ntop(AF_INET, &pAddr->sin_addr, buffer, 1024);
            }
        }
        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);
}

int main(int ac, char** av) {
    int socket_fd = init_socket();
    char src[1024];
    get_src_address(src);
    char *address = av[1];
    struct sockaddr_in *dst = (struct sockaddr_in *)dns_resolution(address);
    printf("PING %s (%s): %lu data bytes\n", address, inet_ntoa(dst->sin_addr), 64 - sizeof(struct icmphdr));
    printf("From %s\n", src);
    // INIT RECV
    char buffer[1024];
    memset(buffer, 0, 1024);

    // STATISTICS INIT
    struct timeval start;
    statistics stat;
    int seq = 0;

    for (;seq < 10; seq++) {

        // INIT PACKET
        packet ping = set_packet(35, seq, src, inet_ntoa(dst->sin_addr));
        // TIME INIT
        gettimeofday(&start, 0);
        // SEND TO
        sendto(socket_fd, &ping, sizeof(struct packet), 0, (struct sockaddr *)dst, sizeof(struct sockaddr_in));
        // RCV MSG
        size_t rec = recvfrom(socket_fd, buffer, 1024, 0, 0, 0);
        packet * pong = (struct packet*)buffer;
        // TIME COMPARAISON
        struct timeval end;
        gettimeofday(&end, 0);
        float diff = timeval_to_float(end) - timeval_to_float(start);

        // DISPLAY
        display_response(pong, diff);

        // UPDATE STATISTICS
        update_stat(&stat, 1, diff);
        usleep(300000);
    }
    // DISPLAY STATISTICS
    printf("--- %s ping statistics ---\n", address);
    display_statistics(stat);
}