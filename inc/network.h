#pragma once
#include <args.h>
#include <packet.h>
#include <sys/socket.h>
#include <netinet/in.h>

void get_src_address(char *buffer);
void setup_network(t_args *args, int *socket_fd, char *src, struct sockaddr_in **dst, int pid);
int send_and_receive_ping(int socket_fd, packet *ping, struct sockaddr_in *dst, char *buffer, float *rtt, int *received_bytes);
