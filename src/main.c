#include <main.h>

int main(int ac, char** av) {
    int socket_fd = init_socket();

    char *address = av[1];
    struct sockaddr_in *dst = (struct sockaddr_in *)dns_resolution(address);
    printf("PING %s (%s): %lu data bytes\n", address, inet_ntoa(dst->sin_addr), 64 - sizeof(struct icmphdr));

    // INIT RECV
    char buffer[1024];

    // STATISTICS INIT
    struct timeval start;
    statistics stat;
    int i = 0;

    for (;i < 10; i++) {

        // INIT PACKET
        packet packet = set_packet(55, i);

        // TIME INIT
        gettimeofday(&start, 0);
        // SEND TO
        sendto(socket_fd, &packet, sizeof(struct packet), 0, (struct sockaddr *)dst, sizeof(struct sockaddr_in));
        // RCV MSG
        recvfrom(socket_fd, buffer, 1024, 0, 0, 0);
        packet_response * response = (struct packet_r *)buffer;
        // TIME COMPARAISON
        struct timeval end;
        gettimeofday(&end, 0);
        float diff = timeval_to_float(end) - timeval_to_float(start);

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