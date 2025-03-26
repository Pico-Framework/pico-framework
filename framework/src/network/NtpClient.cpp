/**
 * @file NtpClient.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "NtpClient.h"
#include "Network.h"
#include <cstdio>
#include <cstring>
#include <functional>
//#include "PicoTime.h"
#include "hardware/rtc.h"
#include "utility.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "pico/time.h"

NTPClient::NTPClient() {
    TRACE("NTPClient initialized, rtc_init\n");
    rtc_init();
}

void NTPClient::setCallback(const std::function<void(time_t)>& cb) {  // ✅ Match header signature
    callback = cb;  // ✅ Store the callback function
}

void NTPClient::requestTime() {
    TRACE("Requesting NTP time...\n");
    if (!Network::isConnected()) {  
        TRACE("Network is not ready. Delaying NTP request...\n");
        return;
    }
    // ✅ Set a default NTP server IP manually (pool.ntp.org → 129.6.15.28)
    IP4_ADDR(&ntp_server, 129, 6, 15, 28);  
    char ntp_ip[16];
    ipaddr_ntoa_r(&ntp_server, ntp_ip, sizeof(ntp_ip));
    TRACE("NTP server IP: %s\n", ntp_ip);

    if (!udp) {
        TRACE("Creating UDP control block...\n");
        udp = udp_new();
        if (!udp) {
            printf("Failed to allocate UDP control block\n");
            return;
        }
        TRACE("Registering UDP receive callback...\n");
        udp_recv(udp, ntpRecv, this);
    }

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
    if (!p) {
        printf("Failed to allocate pbuf\n");
        return;
    }

    uint8_t *msg = static_cast<uint8_t *>(p->payload);
    memset(msg, 0, NTP_MSG_LEN);
    msg[0] = 0b11100011; // LI = 0, Version = 4, Mode = 3 (Client)

    printf("Requesting time from NTP server...\n");
    err_t err = udp_sendto(udp, p, &ntp_server, NTP_PORT);
    pbuf_free(p);

    if (err != ERR_OK) {
        printf("Failed to send NTP request, error: %d\n", err);
    } else {
        printf("NTP request sent successfully!\n");
    }
}

void NTPClient::ntpRecv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    TRACE("NTP response received\n");

    if (!p || p->tot_len < NTP_MSG_LEN) {
        printf("Invalid NTP response (Packet size: %d)\n", p ? p->tot_len : 0);
        if (p) pbuf_free(p);
        return;
    }

    NTPClient *self = static_cast<NTPClient *>(arg);
    self->processResponse(p);
    pbuf_free(p);
}

void NTPClient::processResponse(struct pbuf *p) {
    TRACE("Processing NTP response...\n");
    uint8_t ntp_msg[NTP_MSG_LEN] = {0};
    pbuf_copy_partial(p, ntp_msg, NTP_MSG_LEN, 0);

    uint32_t seconds_since_1900 =
        (ntp_msg[40] << 24) | (ntp_msg[41] << 16) | (ntp_msg[42] << 8) | ntp_msg[43];

    if (seconds_since_1900 == 0) {
        printf("NTP server returned zero timestamp\n");
        return;
    }

    time_t epoch = seconds_since_1900 - NTP_DELTA;
    TRACE("Epoch time received: %ld\n", epoch);

    setSystemTime(epoch);
    TRACE("System time set from NTP\n");

    // ✅ Call the callback if set
    if (callback) {
        callback(epoch);
    }
}

bool testSystemTime(){
    NTPClient ntpClient;
    time_t epoch = 1614556800;  // 2021-03-01 00:00:00
    timeval tv = {epoch, 0};
    settimeofday(&tv, NULL);
    timeval ntv;
    gettimeofday(&ntv, NULL);
    time_t sec = ntv.tv_sec;
    struct tm *tm_info;
    char buffer[30];
  
    tm_info = gmtime(&sec);
  
    strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", tm_info);

    printf("System time set to: %s\n", buffer);
    if (!(ntv.tv_sec >= epoch)) {
        printf("Failed to set system time\n");
        return false;
    }
    else{
        printf("System time set\n");
    }
    return true;
}

bool testDateTime(){
    datetime_t t = {
        .year = 2025,
        .month = 3,
        .day = 1,
        .dotw = 5, // 0 is Sunday, so 5 is Friday
        .hour = 0,
        .min = 0,
        .sec = 0,
    };
    //rtc_init();
    if(rtc_set_datetime(&t))
    {
        printf("RTC time set\n");
    }
    else
    {
        printf("RTC time not set\n");
    }
    datetime_t rtc_time;
    if(!rtc_get_datetime(&rtc_time)) {
        printf("Failed to get RTC time\n");
        return false;
    }
    printf("Actual RTC time updated: %04d-%02d-%02d %02d:%02d:%02d\n",
           rtc_time.year, rtc_time.month, rtc_time.day, rtc_time.hour, rtc_time.min, rtc_time.sec);
    // compare the datetimes
    if (rtc_time.year == t.year && rtc_time.month == t.month && rtc_time.day == t.day && rtc_time.hour == t.hour && rtc_time.min == t.min && rtc_time.sec >= t.sec){
        return true;
    }
    else{
        return false;
    }
}

void NTPClient::setSystemTime(time_t epoch) {
   TRACE("Setting system time to epoch: %lld\n", static_cast<long long>(epoch));

    timeval tv = {epoch, 0};
    settimeofday(&tv, NULL);
    timeval ntv;
    gettimeofday(&ntv, NULL);
    time_t sec = ntv.tv_sec;
    struct tm *timeinfo;
    char buffer[30];
    
    timeinfo = gmtime(&sec);
    
    strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", timeinfo);

    printf("System time set to: %s\n", buffer);
    if (!(ntv.tv_sec >= epoch)) {
        TRACE("epoch is invalid\n");
        return;
    }
    else{
        TRACE("epoch is valid\n");
    }

    // Convert to datetime_t (RTC format)
    datetime_t t = {
        .year  = static_cast<int16_t>(timeinfo->tm_year + 1900),
        .month = static_cast<int8_t>(timeinfo->tm_mon + 1),
        .day   = static_cast<int8_t>(timeinfo->tm_mday),
        .hour  = static_cast<int8_t>(timeinfo->tm_hour),
        .min   = static_cast<int8_t>(timeinfo->tm_min),   
        .sec   = static_cast<int8_t>(timeinfo->tm_sec)
    };

    // Set RTC Time
    //rtc_init();
    if (!rtc_set_datetime(&t)) {
        printf("Failed to set RTC time\n");
        return;
    }
    sleep_us(64);  // Wait for RTC to update

    // Verify RTC Time
    datetime_t rtc_time;
    if (!rtc_get_datetime(&rtc_time)) {
        printf("Failed to get RTC time\n");
        return;
    }

    printf("RTC time set: %04d-%02d-%02d %02d:%02d:%02d\n",
           rtc_time.year, rtc_time.month, rtc_time.day, rtc_time.hour, rtc_time.min, rtc_time.sec);

    //PicoTime::getInstance().printSystemTime();
}





