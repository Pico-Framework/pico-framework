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
    // Minimal definition for tests
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

#if defined(UNIT_TEST) || defined(__UNIT_TEST_HEADER__)
inline std::string& __test_lwip_buffer() {
    static std::string buffer;
    return buffer;
}

inline int lwip_send(int, const void* data, size_t size, int) {
    __test_lwip_buffer().append(static_cast<const char*>(data), size);
    return static_cast<int>(size);
}
#else
inline int lwip_send(int, const void*, size_t, int) { return 0; }
#endif

inline int lwip_recv(int, void*, size_t, int) { return 0; }
inline int lwip_close(int) { return 0; }

inline int lwip_getpeername(int sock, sockaddr* addr, socklen_t* len) {
    return 0;
}
