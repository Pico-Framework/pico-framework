/**
 * @file lwipopts.h
 * @brief Configuration options for the lwIP stack.
 *
 * This configuration is tuned for use with FreeRTOS and Raspberry Pi Pico W (RP2040),
 * incorporating custom memory, TCP, debugging, and threading settings.
 *
 */
#include "stdint.h"

extern void sntp_set_system_time(uint32_t sec);

#ifndef LWIPOPTS_H
#define LWIPOPTS_H

// -----------------------------------------------------------------------------
// System and Core Configuration
// -----------------------------------------------------------------------------
#define NO_SYS 0               // 0: use operating system (FreeRTOS)
#define LWIP_TIMERS 1          // Enable lwIP timer support
#define SYS_LIGHTWEIGHT_PROT 1 // Enable lightweight protection for critical regions

// -----------------------------------------------------------------------------
// Memory Management
// -----------------------------------------------------------------------------

#define MEM_LIBC_MALLOC 0         // Do not use libc malloc
#define MEMP_MEM_MALLOC 0         // Use static memory pools rather than malloc for internal allocations
#define MEM_ALIGNMENT 4           // Align memory to 4-byte boundaries
#ifdef PICO_RP2350
#define MEM_SIZE (16 * 1024)         // Total heap size available to lwIP (4 KB); prevents silent issues at lower sizes
#else
#define MEM_SIZE (10 * 1024)      // Total heap size available to lwIP (16 KB); prevents silent issues at lower sizes [may be able to lower]
#endif
#define MEMP_NUM_NETCONN 32       // Maximum number of simultaneously active netconns (was 32)
#define MEMP_NUM_TCP_PCB 16       // Maximum number of concurrently active TCP protocol control blocks
#define MEMP_NUM_TCP_PCB_LISTEN 8 // Maximum number of listening TCP PCBs
#define MEMP_NUM_TCP_SEG 32       // Maximum number of simultaneously queued TCP segments
#define MEMP_NUM_ARP_QUEUE 10     // Maximum number of ARP request queue entries
#define MEMP_NUM_NETBUF 16        // Maximum number of network buffers
#define MEMP_NUM_SYS_TIMEOUT 16   // Maximum number of active timeouts (old value; increased from 10)
#define NUM_MEMP_PBUF 16          // Number of pbuf structures in the pool

// #define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT 1 // Allow freeing memory allocated in different contexts

// -----------------------------------------------------------------------------
// PBUF Buffer Settings
// -----------------------------------------------------------------------------
#define PBUF_POOL_SIZE 24      // Total number of pbufs in the pool
#define PBUF_POOL_BUFSIZE 1460 // Size (in bytes) of each pbuf in the pool - sane as TCP_MSS

#define ETH_PAD_SIZE 0 // Extra padding added to ethernet frames (0 means no extra pad)

// -----------------------------------------------------------------------------
// TCP Configuration
// -----------------------------------------------------------------------------
#define LWIP_TCP 1                // Enable TCP support
#define LWIP_IPV4 1               // Enable IPv4 support
#define TCP_TTL 255               // Set default Time-To-Live for TCP packets (max hops)
#define TCP_QUEUE_OOSEQ 0         // Disable queuing of out-of-order TCP segments
#define TCP_MSS 1460              // Maximum segment size (bytes)
#define TCP_SND_BUF (8 * TCP_MSS) // Size of TCP sender buffer (bytes) - may be able to lower
// #define TCP_WND (PBUF_POOL_SIZE * (PBUF_POOL_BUFSIZE - 60))
#define TCP_WND (8 * TCP_MSS)                                            // Size of TCP receive window (bytes) - may be able to lower
#define TCP_SND_QUEUELEN ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / TCP_MSS) // Calculate send queue length
#define TCP_SND_QUEUELEN 32                                              // Calculate send queue length
#define TCP_LISTEN_BACKLOG 1                                             // Enable support for backlog on TCP listen
#define TCP_MSL 1000                                                     // Maximum Segment Lifetime (ms) [reduces TIME_WAIT duration]
#define TCP_SYNMAXRTX 3                                                  // Maximum retransmissions for SYN segments

// TCP Keepalive settings:
#define LWIP_TCP_KEEPALIVE 1       // Enable TCP keepalive functionality
#define TCP_KEEPIDLE_DEFAULT 10000 // Time (ms) before sending keepalive probes 
#define TCP_KEEPINTVL_DEFAULT 1000 // Interval (ms) between individual keepalive probes (was 2000)
#define TCP_KEEPCNT_DEFAULT 3      // Maximum number of keepalive probes before closing the connection (was 5)

// TCP additional features:
#define LWIP_TCP_ABORT 1    // Enable immediate TCP abort to drop connections quickly
#define LWIP_TCP_FASTOPEN 1 // Enable TCP Fast Open feature

// -----------------------------------------------------------------------------
// UDP Configuration
// -----------------------------------------------------------------------------
#define LWIP_UDP 1  // Enable UDP support - needed for NTP time sync
#define UDP_TTL 255 // Default Time-To-Live for UDP packets

// -----------------------------------------------------------------------------
// ICMP / DNS / DHCP / AUTOIP / SNTP
// -----------------------------------------------------------------------------
#define LWIP_ICMP 1             // Enable ICMP (e.g., ping)
#define LWIP_DNS 1              // Enable DNS resolution
#define DNS_TABLE_SIZE 4        // Maximum DNS table entries
#define DNS_MAX_NAME_LENGTH 256 // Maximum domain name length for DNS
#define DNS_MAX_SERVERS 2       // Maximum number of DNS servers
#define LWIP_DHCP 1             // Enable DHCP client support
#define LWIP_AUTOIP 0           // Disable AutoIP (self-assigned IP addressing)
#define LWIP_SNTP 1             // Enable SNTP (Simple Network Time Protocol)
#define SNTP_SUPPORT 1          // Enable SNTP support
#define SNTP_CHECK_RESPONSE 1 // Enable SNTP response checking
#define SNTP_RETRY_TIMEOUT 5000 // Retry timeout for SNTP requests (in ms)
#define SNTP_SERVER_ADDRESS "pool.ntp.org" // Default NTP server address
#define SNTP_SERVER_PORT 123     // Default NTP server port
#define SNTP_SERVER_DNS 1       // Enable DNS server for SNTP
#define SNTP_MAX_SERVERS 1      // Maximum number of SNTP servers
#define SNTP_SET_SYSTEM_TIME 1  // Enable setting system time from SNTP
#define SNTP_UPDATE_DELAY 60*60*1000 // Delay (in ms) between SNTP updates - this will set the imer once an hour
// Set system time from SNTP
#define SNTP_SET_SYSTEM_TIME(sec) sntp_set_system_time(sec)

// -----------------------------------------------------------------------------
// Protocols and Raw API
// -----------------------------------------------------------------------------
#define LWIP_RAW 1            // Enable the RAW API for low-level protocol access
#define LWIP_NETCONN 1        // Used by sockets and netconn API
#define LWIP_SOCKET 1         // Enable socket API support
#define LWIP_COMPAT_SOCKETS 0 // Use POSIX-like sockets instead of native lwIP sockets

// -----------------------------------------------------------------------------
// Threading / OS Integration
// -----------------------------------------------------------------------------
#define LWIP_NETCONN_SEM_PER_THREAD 0    // Each thread gets its own netconn semaphore - only if using a single socket from multiple threads
#define LWIP_SO_RCVTIMEO 1               // Enable support for socket receive timeouts
#define LWIP_TCPIP_CORE_LOCKING 1        // Enable core locking for thread safety
#define LWIP_TCPIP_CORE_LOCKING_INPUT 1  // Enable core locking for input processing
#define TCPIP_THREAD_NAME "tcpip_thread" // Name assigned to the TCP/IP thread
#define TCPIP_THREAD_STACKSIZE 2048      // Stack size (in bytes) for the TCP/IP thread
#define TCPIP_THREAD_PRIO 8              // Priority for the TCP/IP thread
#define TCPIP_MBOX_SIZE 8                // Mailbox size for the TCP/IP thread message queue
#define DEFAULT_THREAD_STACKSIZE 2048    // Default stack size for lwIP-related threads
#define DEFAULT_UDP_RECVMBOX_SIZE 8      // Default mailbox size for UDP receive (old value)
#define DEFAULT_TCP_RECVMBOX_SIZE 8      // Default mailbox size for TCP receive
#define DEFAULT_ACCEPTMBOX_SIZE 8        // Default mailbox size for accepted connections
#define DEFAULT_RAW_RECVMBOX_SIZE 8      // Default mailbox size for raw receive (from old settings)
#define LWIP_TIMEVAL_PRIVATE 0           // Use the standard system timeval structure

// -----------------------------------------------------------------------------
// ALTCP / Secure Sockets / TLS Support
// -----------------------------------------------------------------------------
#ifdef PICO_TCP_ENABLE_TLS
#define LWIP_ALTCP 1             // Enable the ALTCP abstraction layer
#define LWIP_ALTCP_TLS 1         // Enable TLS support for ALTCP
#define LWIP_ALTCP_TLS_MBEDTLS 1 // Enable mbedTLS support for ALTCP
#else
#define LWIP_ALTCP 0
#define LWIP_ALTCP_TLS 0
#define LWIP_ALTCP_TLS_MBEDTLS 0
#endif

// -----------------------------------------------------------------------------
// Interface / Ethernet / Link Layer
// -----------------------------------------------------------------------------
#define LWIP_ARP 1                       // Enable ARP (Address Resolution Protocol)
#define LWIP_ETHERNET 1                  // Enable Ethernet support
#define LWIP_NETIF_HOSTNAME 1            // Allow network interfaces to have hostnames
#define LWIP_NETIF_STATUS_CALLBACK 1     // Enable callbacks on network interface status changes
#define LWIP_NETIF_LINK_CALLBACK 1       // Enable callbacks on network link state changes
#define LWIP_NETIF_EXT_STATUS_CALLBACK 1 // Enable extended status callbacks for network interfaces
#define DHCP_DOES_ARP_CHECK 0            // Disable ARP check for DHCP responses
#define LWIP_DHCP_DOES_ACD_CHECK 0       // Disable Address Conflict Detection (ACD) for DHCP
#define LWIP_NETIF_TX_SINGLE_PBUF 1      // Transmit a single pbuf per packet (improves performance)
#define ETHARP_TABLE_SIZE 127            // Set the size of the ARP table

// -----------------------------------------------------------------------------
// Checksum and CRCs
// -----------------------------------------------------------------------------
#define CHECKSUM_GEN_IP 1       // Generate checksums for outgoing IP packets
#define CHECKSUM_GEN_UDP 1      // Generate checksums for outgoing UDP packets
#define CHECKSUM_GEN_TCP 1      // Generate checksums for outgoing TCP packets
#define CHECKSUM_CHECK_IP 1     // Validate checksums on incoming IP packets
#define CHECKSUM_CHECK_UDP 1    // Validate checksums on incoming UDP packets
#define CHECKSUM_CHECK_TCP 1    // Validate checksums on incoming TCP packets
#define LWIP_CHKSUM_ALGORITHM 3 // Select the checksum algorithm (3 = optimized algorithm)

// -----------------------------------------------------------------------------
// Statistics and Debugging
// -----------------------------------------------------------------------------
#define LWIP_STATS 1         // Enable collection of lwIP statistics
#define LWIP_STATS_DISPLAY 1 // Enable display routines for statistics
#define LWIP_PERF 0          // Disable performance testing macros

// -----------------------------------------------------------------------------
// Debugging (Individual modules only - LWIP_DEBUG must remain undefined!)
// See https://savannah.nongnu.org/bugs/index.php?62159
// -----------------------------------------------------------------------------
#define LWIP_DEBUG 1 // Globally disable debug output to avoid savannah bug (fix applied)
//#undef LWIP_DEBUG    // Ensure the global debug macro is undefined
#ifndef NDEBUG
// Enable detailed debug flags only if not in release mode and global debug is off (workaround for savannah bug)
#ifndef LWIP_DEBUG
#define ETHARP_DEBUG LWIP_DBG_OFF
#define NETIF_DEBUG LWIP_DBG_OFF
#define PBUF_DEBUG LWIP_DBG_OFF
#define API_LIB_DEBUG LWIP_DBG_OFF
#define API_MSG_DEBUG LWIP_DBG_OFF
#define SOCKETS_DEBUG LWIP_DBG_OFF
#define ICMP_DEBUG LWIP_DBG_OFF
#define INET_DEBUG LWIP_DBG_OFF
#define IP_DEBUG LWIP_DBG_OFF
#define IP_REASS_DEBUG LWIP_DBG_OFF
#define RAW_DEBUG LWIP_DBG_OFF
#define MEM_DEBUG LWIP_DBG_OFF
#define MEMP_DEBUG LWIP_DBG_OFF
#define SYS_DEBUG LWIP_DBG_OFF
#define TCP_DEBUG LWIP_DBG_OFF
#define TCP_INPUT_DEBUG LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG LWIP_DBG_OFF
#define TCP_RTO_DEBUG LWIP_DBG_OFF
#define TCP_CWND_DEBUG LWIP_DBG_OFF
#define TCP_WND_DEBUG LWIP_DBG_OFF
#define TCP_FR_DEBUG LWIP_DBG_OFF
#define TCP_QLEN_DEBUG LWIP_DBG_OFF
#define TCP_RST_DEBUG LWIP_DBG_OFF
#define UDP_DEBUG LWIP_DBG_OFF
#define TCPIP_DEBUG LWIP_DBG_OFF
#define PPP_DEBUG LWIP_DBG_OFF
#define SLIP_DEBUG LWIP_DBG_OFF
#define DHCP_DEBUG LWIP_DBG_OFF
#define SNTP_DEBUG LWIP_DBG_ON
#endif
#endif
#define ALTCP_MBEDTLS_DEBUG LWIP_DBG_OFF // Disable debugging for ALTCP mbedTLS

#endif /* LWIPOPTS_H */
