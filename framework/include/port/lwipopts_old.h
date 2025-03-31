#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Generally you would define your own explicit list of lwIP options
// (see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html)
//
#ifndef _LWIPOPTS_EXAMPLE_COMMONH_H
#define _LWIPOPTS_EXAMPLE_COMMONH_H

// Common settings used in most of the pico_w examples
// (see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html for details)

// allow override in some examples
#ifndef NO_SYS
#define NO_SYS                      0
#endif

// had to add these to avoid a PICO SDK ASSERT size  > 0
#define DEFAULT_UDP_RECVMBOX_SIZE 8
#define DEFAULT_TCP_RECVMBOX_SIZE 127
#define DEFAULT_ACCEPTMBOX_SIZE 127
//

// use explicit calls to lwip for bind/accept/send etc. because of conlicts iwith json...
#define LWIP_COMPAT_SOCKETS 0


#ifndef LWIP_SOCKET
#define LWIP_SOCKET                 1
#endif
#if PICO_CYW43_ARCH_POLL
#define MEM_LIBC_MALLOC             1
#else
// MEM_LIBC_MALLOC is incompatible with non polling versions
#define MEM_LIBC_MALLOC             0 
#endif
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    16* 1024 // Was causing various silent problems at 8192!

#define MEMP_NUM_TCP_PCB            32  // I cut this back from 64 to 32 to save memory
#define MEMP_NUM_TCP_PCB_LISTEN     16

#define MEMP_MEM_MALLOC             0   // this should be checked - we started with 0 using pools
#define NUM_MEMP_PBUF               16  // trying to fix accept issue
#define MEMP_NUM_NETBUF             16  // trying to fix accept issue
#define MEMP_NUM_NETCONN            32  // this fixes accept ERR_ABRT
#define MEMP_NUM_SYS_TIMEOUT        16  // trying to fix accept issue

#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_ARP_QUEUE          10
#define PBUF_POOL_SIZE              24
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    0   // we're not using the raw api
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_MSS                     1460
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_NETCONN                0
#define MEM_STATS                   1
#define SYS_STATS                   1
#define MEMP_STATS                  1
#define LINK_STATS                  1
// #define ETH_PAD_SIZE                2
#define LWIP_CHKSUM_ALGORITHM       3
#define LWIP_DHCP                   1
#define LWIP_IPV4                   1
#define LWIP_TCP                    1
#define LWIP_TCP_ABORT              1
#define LWIP_UDP                    1
#define LWIP_DNS                    1
#define LWIP_TCP_KEEPALIVE          1
#define TCP_MSL                     1000 // Reduce TIME_WAIT timeout (default is 60 sec)
#define TCP_SYNMAXRTX               3
#define TCP_TTL                     5
#define TCP_KEEPIDLE                2000      // 2s idle before keepalive probes
#define TCP_KEEPCNT                 3         // 3 keepalive probes before close
#define TCP_KEEPINTVL               1000      // 1s between keepalive probes
#define LWIP_TCP_FASTOPEN           1
#define LWIP_SO_REUSE               0
//#define TCP_KEEPALIVE_TIMEOUT      60 // trying to fix -13 errors on accept
//#define TCP_FIN_WAIT_TIMEOUT        5 // trying to fix -13 errors on acceptl
#define LWIP_NETIF_TX_SINGLE_PBUF   1
#define DHCP_DOES_ARP_CHECK         0
#define LWIP_DHCP_DOES_ACD_CHECK    0

#define TCP_QUEUE_OOSEQ             0  // Disable out-of-order queuing (if enabled)
#define LWIP_NETCONN_SEM_PER_THREAD 1  // Each thread has its own netconn semaphore

#ifndef NDEBUG
#define LWIP_DEBUG                  1
#define LWIP_STATS                  1
#define LWIP_STATS_DISPLAY          1
#endif

#define ETHARP_TABLE_SIZE           127

#define ETHARP_DEBUG                LWIP_DBG_ON
#define NETIF_DEBUG                 LWIP_DBG_ON
#define PBUF_DEBUG                  LWIP_DBG_ON
#define API_LIB_DEBUG               LWIP_DBG_ON
#define API_MSG_DEBUG               LWIP_DBG_ON
#define SOCKETS_DEBUG               LWIP_DBG_ON
#define ICMP_DEBUG                  LWIP_DBG_ON
#define INET_DEBUG                  LWIP_DBG_ON
#define IP_DEBUG                    LWIP_DBG_ON
#define IP_REASS_DEBUG              LWIP_DBG_ON
#define RAW_DEBUG                   LWIP_DBG_ON
#define MEM_DEBUG                   LWIP_DBG_ON
#define MEMP_DEBUG                  LWIP_DBG_ON
#define SYS_DEBUG                   LWIP_DBG_ON
#define TCP_DEBUG                   LWIP_DBG_ON
#define TCP_INPUT_DEBUG             LWIP_DBG_ON
#define TCP_OUTPUT_DEBUG            LWIP_DBG_ON
#define TCP_RTO_DEBUG               LWIP_DBG_ON
#define TCP_CWND_DEBUG              LWIP_DBG_ON
#define TCP_WND_DEBUG               LWIP_DBG_ON
#define TCP_FR_DEBUG                LWIP_DBG_ON
#define TCP_QLEN_DEBUG              LWIP_DBG_ON
#define TCP_RST_DEBUG               LWIP_DBG_ON
#define UDP_DEBUG                   LWIP_DBG_ON
#define TCPIP_DEBUG                 LWIP_DBG_ON
#define PPP_DEBUG                   LWIP_DBG_ON
#define SLIP_DEBUG                  LWIP_DBG_ON
#define DHCP_DEBUG                  LWIP_DBG_ON

#endif /* __LWIPOPTS_H__ */


#if !NO_SYS
#define TCPIP_THREAD_STACKSIZE         2048 // cut from 8192 to save memory but not sure it was used anyway

#define DEFAULT_THREAD_STACKSIZE       2048 // increase from 1024 to avoid stack overflow - this being used by this port for lwip
#define DEFAULT_RAW_RECVMBOX_SIZE      8
#define TCPIP_MBOX_SIZE                127
#define LWIP_TIMEVAL_PRIVATE           0

// not necessary, can be done either way
#define LWIP_TCPIP_CORE_LOCKING_INPUT 0 // turn off core locking for stability

// ping_thread sets socket receive timeout, so enable this feature
#define LWIP_SO_RCVTIMEO              1
#endif

#define LWIP_ALTCP 1

// If you don't want to use TLS (just a http request) you can avoid linking to mbedtls and remove the following
#define LWIP_ALTCP_TLS           1
#define LWIP_ALTCP_TLS_MBEDTLS   1

// Note bug in lwip with LWIP_ALTCP and LWIP_DEBUG
// https://savannah.nongnu.org/bugs/index.php?62159
#define LWIP_DEBUG 1
#undef LWIP_DEBUG
//#define ALTCP_MBEDTLS_DEBUG  LWIP_DBG_ON

#endif