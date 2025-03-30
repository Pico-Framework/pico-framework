#pragma once

#ifndef LWIP_SOCKETS_H
#define LWIP_SOCKETS_H

#ifdef __APPLE__
// On macOS, rely on system definitions.
#include <sys/socket.h>
#else
// On non-macOS platforms, define the types that lwIP normally provides.
typedef int socklen_t;

struct sockaddr {
    // Provide a minimal definition if your tests need it.
};

struct in_addr {
    unsigned long s_addr;
};

struct sockaddr_in {
    unsigned char sin_len;
    unsigned char sin_family;
    unsigned short sin_port;
    struct in_addr sin_addr;
    char sin_zero[8];
};

#endif  // __APPLE__

#endif  // LWIP_SOCKETS_H


// #include <sys/types.h>  // includes ssize_t on macOS
// #include <cstdint>
// #include <cstring>

// typedef int socklen_t;

// struct in_addr {
//     uint32_t s_addr;
// };



// struct sockaddr {
//     uint16_t sa_family;
//     char sa_data[14];
// };


inline int lwip_recv(int, void*, size_t, int) { return 0; }
inline int lwip_send(int, const void*, size_t, int) { return 0; }
inline int lwip_close(int) { return 0; }  // <--- Add this line


// struct sockaddr_in {
//     int sin_family;
//     uint16_t sin_port;
//     //in_addr sin_addr;
// };

inline int lwip_getpeername(int sock, sockaddr* addr, socklen_t* len) {
    // if (addr && len) {
    //     sockaddr_in* in = reinterpret_cast<sockaddr_in*>(addr);
    //     in->sin_addr.s_addr = 0xC0A80001; // 192.168.0.1
    // }
    return 0;
}