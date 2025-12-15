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
    struct ip header_ip;
    struct icmp header_icmp;
    char msg[PACKET_SIZE - sizeof(struct icmp) - sizeof(struct ip)];
} packet;

uint16_t check_sum(const void* data, size_t len);
struct sockaddr_in set_sockaddr_in(char *addr);
packet set_packet(int id, int seq, char *src, char *dst);
packet set_packet_with_ttl(int id, int seq, char *src, char *dst, int ttl);
struct msghdr set_response();
void display_response(packet * response, float time);
struct sockaddr * dns_resolution(char *fqdn);
int init_socket();
void print_icmp_response(unsigned char* buffer, int bytes_received);