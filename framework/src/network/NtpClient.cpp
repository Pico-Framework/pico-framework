/**
 * @file NtpClient.cpp
 * @author Ian Archbell
 * @brief Implementation of a DNS-based NTP client with retry and RTC update.
 *
 * part of the PicoFramework application framework.
 * Handles DNS resolution of public NTP servers, sends UDP request,
 * retries on failure, and sets both system and RTC time.
 *
 * @version 0.2
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #define TRACE_MODULE "NET"
 #define TRACE_ENABLED false
 #include "DebugTrace.h"

 #include "NtpClient.h"
 #include "Network.h"
 #include "hardware/rtc.h"
 //#include "utility.h"
 #include "lwip/dns.h"
 #include "DebugTrace.h"
 
 #define NTP_MSG_LEN 48
 #define NTP_PORT 123
 #define NTP_DELTA 2208988800UL // 1900 to 1970 epoch difference
 
 /// @copydoc NTPClient::NTPClient()
 NTPClient::NTPClient() {
     TRACE("NTPClient initialized\n");
     rtc_init();
 }
 
 /// @copydoc NTPClient::setCallback()
 void NTPClient::setCallback(const std::function<void(time_t)>& cb) {
     callback = cb;
 }
 
 /// @copydoc NTPClient::requestTime()
 void NTPClient::requestTime() {
     retryCount = 0;
     currentServerIndex = 0;
     resolveAndSendRequest(ntpPool[currentServerIndex]);
 }
 
 /// @copydoc NTPClient::resolveAndSendRequest()
 void NTPClient::resolveAndSendRequest(const char* hostname) {
     TRACE("Resolving NTP host: %s\n", hostname);
 
     ip_addr_t resolved;
     err_t err = dns_gethostbyname(hostname, &resolved, dnsCallback, this);
 
     if (err == ERR_OK) {
         TRACE("DNS resolved immediately\n");
         sendNtpRequest(&resolved);
     } else if (err != ERR_INPROGRESS) {
         printf("DNS lookup failed for %s: %d\n", hostname, err);
         retry();
     }
 }
 
 /// @copydoc NTPClient::dnsCallback()
 void NTPClient::dnsCallback(const char* name, const ip_addr_t* ipaddr, void* arg) {
     NTPClient* self = static_cast<NTPClient*>(arg);
     if (!ipaddr) {
         printf("DNS failed for %s\n", name);
         self->retry();
         return;
     }
     self->sendNtpRequest(ipaddr);
 }
 
 /// @copydoc NTPClient::sendNtpRequest()
 void NTPClient::sendNtpRequest(const ip_addr_t* server) {
     TRACE("Sending NTP request to %s\n", ipaddr_ntoa(server));
 
     if (!Network::isConnected()) {
         printf("Network not ready\n");
         retry();
         return;
     }
 
     if (!udp) {
         udp = udp_new();
         if (!udp) {
             printf("Failed to allocate UDP PCB\n");
             retry();
             return;
         }
         udp_recv(udp, ntpRecv, this);
     }
 
     struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
     if (!p) {
         printf("Failed to allocate pbuf\n");
         retry();
         return;
     }
 
     uint8_t* msg = static_cast<uint8_t*>(p->payload);
     memset(msg, 0, NTP_MSG_LEN);
     msg[0] = 0b11100011; // LI=3 (unsync), VN=4, Mode=3 (Client)
 
     err_t err = udp_sendto(udp, p, server, NTP_PORT);
     pbuf_free(p);
 
     if (err != ERR_OK) {
         printf("UDP send failed: %d\n", err);
         retry();
     } else {
         TRACE("NTP request sent\n");
     }
 }
 
 /// @copydoc NTPClient::ntpRecv()
 void NTPClient::ntpRecv(void* arg, struct udp_pcb* pcb, struct pbuf* p,
                         const ip_addr_t* addr, u16_t port) {
     if (!p || p->tot_len < NTP_MSG_LEN) {
         printf("Invalid NTP response\n");
         if (p) pbuf_free(p);
         return;
     }
 
     NTPClient* self = static_cast<NTPClient*>(arg);
     self->processResponse(p);
     pbuf_free(p);
 }
 
 /// @copydoc NTPClient::processResponse()
 void NTPClient::processResponse(struct pbuf* p) {
     uint8_t ntp_msg[NTP_MSG_LEN] = {0};
     pbuf_copy_partial(p, ntp_msg, NTP_MSG_LEN, 0);
 
     uint32_t seconds_since_1900 =
         (ntp_msg[40] << 24) | (ntp_msg[41] << 16) |
         (ntp_msg[42] << 8)  | ntp_msg[43];
 
     if (seconds_since_1900 == 0) {
         printf("NTP server returned zero timestamp\n");
         retry();
         return;
     }
 
     time_t epoch = seconds_since_1900 - NTP_DELTA;
     TRACE("Received epoch: %lu\n", epoch);
 
     setSystemTime(epoch);
 
     if (callback) {
         callback(epoch);
     }
 }
 
 /// @copydoc NTPClient::setSystemTime()
 void NTPClient::setSystemTime(time_t epoch) {
     timeval tv = {epoch, 0};
     settimeofday(&tv, NULL);
 
     struct timeval current;
     gettimeofday(&current, NULL);
     struct tm* tm_now = gmtime(&current.tv_sec);
 
     datetime_t rtc_time = {
         .year  = (int16_t)(tm_now->tm_year + 1900),
         .month = (int8_t)(tm_now->tm_mon + 1),
         .day   = (int8_t)(tm_now->tm_mday),
         .hour  = (int8_t)(tm_now->tm_hour),
         .min   = (int8_t)(tm_now->tm_min),
         .sec   = (int8_t)(tm_now->tm_sec)
     };
 
     rtc_set_datetime(&rtc_time);
 
     printf("System and RTC time set: %04d-%02d-%02d %02d:%02d:%02d\n",
            rtc_time.year, rtc_time.month, rtc_time.day,
            rtc_time.hour, rtc_time.min, rtc_time.sec);
 }
 
 /// @copydoc NTPClient::retry()
 void NTPClient::retry() {
     if (++retryCount >= maxRetries) {
         printf("Max NTP retries reached. Giving up.\n");
         return;
     }
 
     currentServerIndex = (currentServerIndex + 1) % poolSize;
     resolveAndSendRequest(ntpPool[currentServerIndex]);
 }
 