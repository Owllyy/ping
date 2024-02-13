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

struct icmphdr {
    u_char  type;       // type of message, see below
	u_char  code;       // type sub code
	u_short cksum;      // ones complement cksum of struct
    n_short id;         // sender id
    n_short seq;        // sequence index
};

typedef struct packet {
    struct icmphdr header_icmp;
    char msg[64 - sizeof(struct icmphdr) - sizeof(struct ip)];
} packet;

typedef struct packet_r {
    struct ip header_ip;
    struct icmp header_icmp;
    char msg[64 - sizeof(struct icmp) - sizeof(struct ip)];
} packet_response;

uint16_t check_sum(const void* data, size_t len);
struct sockaddr_in set_sockaddr_in(char *addr);
packet set_packet(int id, int seq);
struct msghdr set_response();
void display_response(packet_response * response, float time);
struct sockaddr * dns_resolution(char *fqdn);
int init_socket();