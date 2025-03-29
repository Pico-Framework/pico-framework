#pragma once

#include <sys/types.h>  // includes ssize_t on macOS
#include <cstdint>
#include <cstring>

typedef int socklen_t;

struct sockaddr {
    uint16_t sa_family;
    char sa_data[14];
};


inline int lwip_recv(int, void*, size_t, int) { return 0; }
inline int lwip_send(int, const void*, size_t, int) { return 0; }
inline int lwip_close(int) { return 0; }  // <--- Add this line

struct in_addr {
    uint32_t s_addr;
};

struct sockaddr_in {
    int sin_family;
    uint16_t sin_port;
    in_addr sin_addr;
};

inline int lwip_getpeername(int sock, sockaddr* addr, socklen_t* len) {
    if (addr && len) {
        sockaddr_in* in = reinterpret_cast<sockaddr_in*>(addr);
        in->sin_addr.s_addr = 0xC0A80001; // 192.168.0.1
    }
    return 0;
}