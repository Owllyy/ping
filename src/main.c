#include <main.h>
#define BUFFER_SIZE 1024

int filter(char *buffer) {
    return 0;
}

float timed_recv(int socket_fd, char *buffer) {
    struct timeval start;
    gettimeofday(&start,0);
    int recv_size = 0;

    while (!is_timeout(start, 3000)) {
        recv_size = recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, 0, 0);
        if (recv_size > 0 && !filter(buffer)) {
            return duration_in_ms(start);
        }
        recv_size = 0;
    }
    return -1.0;
}

int main(int ac, char** av) {
    int socket_fd = init_socket();
    char *address = av[1];

    struct sockaddr_in *dst = (struct sockaddr_in *)dns_resolution(address);
    printf("PING %s (%s): %lu data bytes\n", address, inet_ntoa(dst->sin_addr), 64 - sizeof(struct icmphdr));

    // INIT RECV
    char buffer[1024];
    memset(buffer, 0, 1024);
    packet_r * pong = (struct packet_r *)buffer; 

    // STATISTICS INIT
    statistics stat;

    for (int seq = 0 ;seq < 10; seq++) {
        // INIT PACKET
        packet ping = set_packet(35, seq);
        // SEND TO
        sendto(socket_fd, &ping, sizeof(packet), 0, (struct sockaddr *)dst, sizeof(struct sockaddr_in));
        // TIME COMPARAISON
        float diff = timed_recv(socket_fd, buffer);
        // DISPLAY
        if (diff < 0.0) {
            printf("Request timeout for icmp_seq %d\n", seq);
            update_stat(&stat, 0, diff);
        } else {
            display_response(pong, diff);
        } 
        // UPDATE STATISTICS
        update_stat(&stat, 1, diff);
        usleep(300000);
    }
    // DISPLAY STATISTICS
    printf("--- %s ping statistics ---\n", address);
    display_statistics(stat);

}