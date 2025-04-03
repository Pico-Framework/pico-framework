#pragma once

#ifndef DUMMY_IP_ADDR_H
#define DUMMY_IP_ADDR_H

#include <arpa/inet.h>   // For inet_ntoa
#include <netinet/in.h>  // For in_addr

#ifdef __cplusplus
extern "C" {
#endif


// Define a dummy ip_addr_t structure with an 'addr' member.
typedef struct ip_addr {
    unsigned int addr;
} ip_addr_t;

// Stub for ipaddr_ntoa: converts an ip_addr_t to its string representation.
// Note: This uses inet_ntoa which returns a pointer to a static buffer.
static char* ipaddr_ntoa(const ip_addr_t* ip) {
    struct in_addr in;
    in.s_addr = ip->addr;
    return inet_ntoa(in);
}

#ifdef __cplusplus
}
#endif

#endif // DUMMY_IP_ADDR_H
