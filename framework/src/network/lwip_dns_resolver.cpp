#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "pico/time.h"
#include <cstring>
//#include "framework_config.h"
//#include "DebugTrace.h"
//TRACE_INIT(LwipDnsResolver);

static volatile bool dns_done = false;
static ip_addr_t resolved_ip;

static void dns_callback(const char* name, const ip_addr_t* ipaddr, void* callback_arg) {
    if (ipaddr) {
        resolved_ip = *ipaddr;
    } else {
        ip_addr_set_any(IPADDR_TYPE_V4, &resolved_ip);
    }
    dns_done = true;
}

/**
 * @brief Blocking DNS resolution using lwIP.
 * @param hostname The host to resolve.
 * @param result Pointer to an ip_addr_t to store the result.
 * @param timeout_ms Max time to wait for resolution.
 * @return true if successful, false otherwise.
 */
bool resolveHostnameBlocking(const char* hostname, ip_addr_t* result, uint32_t timeout_ms = 5000) {

    printf("[DNS] Starting DNS lookup for %s\n", hostname);

    dns_done = false;
    err_t err = dns_gethostbyname(hostname, &resolved_ip, dns_callback, nullptr);

    if (err == ERR_OK) {
        printf("[DNS] Hostname resolved immediately: %s -> %s\n", hostname, ipaddr_ntoa(&resolved_ip));
        *result = resolved_ip;
        return true;
    } else if (err == ERR_INPROGRESS) {
        printf("[DNS] Lookup in progress for %s, waiting...\n", hostname);
    } else {
        printf("[DNS] dns_gethostbyname failed with error %d\n", err);
        return false;
    }

    // Wait for async resolution
    absolute_time_t deadline = make_timeout_time_ms(timeout_ms);
    while (!dns_done && absolute_time_diff_us(get_absolute_time(), deadline) > 0) {
        sleep_ms(10);
    }

    if (dns_done && !ip_addr_isany(&resolved_ip)) {
        printf("[DNS] DNS resolved after wait: %s -> %s\n", hostname, ipaddr_ntoa(&resolved_ip));
        *result = resolved_ip;
        return true;
    } else {
        printf("[DNS] DNS resolution timed out or returned null\n");
        return false;
    }
}

