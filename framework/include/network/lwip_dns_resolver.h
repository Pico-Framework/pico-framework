#pragma once

#include "lwip/ip_addr.h"
#include <stdint.h>

/**
 * @brief Resolve hostname to IP address using lwIP DNS (blocking).
 *
 * @param hostname The hostname to resolve.
 * @param result Pointer to store resolved IP address.
 * @param timeout_ms Max wait time in milliseconds.
 * @return true if resolved successfully, false otherwise.
 */
bool resolveHostnameBlocking(const char* hostname, ip_addr_t* result, uint32_t timeout_ms = 5000);
