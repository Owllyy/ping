#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#define SEC_PER_DAY   86400
#define SEC_PER_HOUR  3600
#define SEC_PER_MIN   60
 

typedef struct packet {
    struct icmp header_icmp;
    char msg[64 - sizeof(struct icmp)];
} packet;

typedef struct packet_r {
    struct ip header_ip;
    struct icmp header_icmp;
    char msg[64 - sizeof(struct ip) - sizeof(struct icmp)];
} packet_response;

typedef struct statistics {
    unsigned int transmitted;
    unsigned int received;
    struct timeval min;
    struct timeval max;
} statistics;