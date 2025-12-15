#include <network.h>
#include <packet.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <duration.h>

void get_src_address(char *buffer) {
   struct ifaddrs *addrs, *tmp;
    getifaddrs(&addrs);
    tmp = addrs;

    while (tmp) {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            if (strcmp(tmp->ifa_name, "lo") != 0 && !buffer[0]) {
                inet_ntop(AF_INET, &pAddr->sin_addr, buffer, 1024);
            }
        }
        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);
}

void setup_network(t_args *args, int *socket_fd, char *src, struct sockaddr_in **dst) {
    *socket_fd = init_socket();
    get_src_address(src);
    if (!src[0]) {
        fprintf(stderr, "ft_ping: Failed to find network interface\n");
        exit(1);
    }
    *dst = (struct sockaddr_in *)dns_resolution(args->address);
    printf("PING %s (%s): 56 data bytes\n", args->address, inet_ntoa((*dst)->sin_addr));
}

float send_and_receive_ping(int socket_fd, packet *ping, struct sockaddr_in *dst, char *buffer) {
    struct timeval start, end;

    gettimeofday(&start, 0);
    sendto(socket_fd, ping, sizeof(struct packet), 0, (struct sockaddr *)dst, sizeof(struct sockaddr_in));
    recvfrom(socket_fd, buffer, 1024, 0, 0, 0);
    gettimeofday(&end, 0);

    return timeval_to_float(end) - timeval_to_float(start);
}
