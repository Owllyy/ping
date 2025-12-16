#pragma once
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PACKET_SIZE 64

typedef struct packet {
    struct icmp header_icmp;
    char msg[PACKET_SIZE - sizeof(struct icmp)];
} packet;

uint16_t check_sum(const void* data, size_t len);
packet set_packet(int id, int seq);
void display_response(struct ip *ip_hdr, struct icmp *icmp_hdr, float time);
struct sockaddr * dns_resolution(char *fqdn);
int init_socket();
void set_socket_ttl(int socket_fd, int ttl);