#pragma once

#pragma once
#include <cstdint>
#include <string>
#include <cstdio>

// === Mock lwip stats structure ===
struct {
    struct {
        int avail = 1024;
        int used = 512;
    } mem;
} lwip_stats;

// === Mock ip_addr_t and helpers ===
struct ip_addr_t {
    uint32_t addr;
};

inline const char* ipaddr_ntoa(const ip_addr_t* ip) {
    static char buf[16];
    std::snprintf(buf, sizeof(buf), "%u.%u.%u.%u",
                  (ip->addr >> 24) & 0xFF,
                  (ip->addr >> 16) & 0xFF,
                  (ip->addr >> 8) & 0xFF,
                  ip->addr & 0xFF);
    return buf;
}