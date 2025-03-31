/**
 * @file NtpClient.h
 * @author Ian Archbell
 * @brief Header for NTP client to sync time via UDP from public NTP servers.
 * 
 * Part of the PicoFramework application framework.
 * Supports time sync via DNS-resolved NTP pool, retries on failure,
 * and RTC + system time update. Uses lwIP's UDP and DNS APIs.
 *
 * @version 0.2
 * @date 2025-03-31
 * 
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #pragma once

 #include "lwip/udp.h"
 #include "lwip/ip_addr.h"
 #include <functional>
 #include <ctime>
 
 /**
  * @class NTPClient
  * @brief Provides NTP time synchronization using a pool of hostnames and UDP requests.
  *
  * Performs DNS lookup, sends UDP request, parses the NTP response, and sets the system and RTC time.
  * Supports multiple retries across a pool of NTP servers.
  */
 class NTPClient {
 public:
     /**
      * @brief Construct a new NTPClient. Initializes RTC hardware.
      */
     NTPClient();
 
     /**
      * @brief Initiate an NTP time request. Uses DNS to resolve a pool of hostnames.
      */
     void requestTime();
 
     /**
      * @brief Register a callback to be called after the time is successfully updated.
      * @param cb A function accepting `time_t` (epoch time) as input.
      */
     void setCallback(const std::function<void(time_t)>& cb);
 
 private:
     udp_pcb* udp = nullptr;
     std::function<void(time_t)> callback;
 
     int retryCount = 0;          ///< Tracks retry attempts
     const int maxRetries = 3;    ///< Maximum number of retry attempts
     int currentServerIndex = 0;  ///< Index into the pool of NTP servers
 
     static constexpr const char* ntpPool[] = {
         "pool.ntp.org",
         "time.google.com",
         "time.windows.com",
         "time.nist.gov"
     };
     static constexpr int poolSize = sizeof(ntpPool) / sizeof(ntpPool[0]);
 
     /**
      * @brief Resolve the given hostname and attempt to send the request.
      * @param hostname NTP server hostname
      */
     void resolveAndSendRequest(const char* hostname);
 
     /**
      * @brief lwIP DNS callback when resolution completes.
      * @param name Hostname resolved
      * @param ipaddr Resulting IP address
      * @param arg Pointer to this NTPClient
      */
     static void dnsCallback(const char* name, const ip_addr_t* ipaddr, void* arg);
 
     /**
      * @brief Send an NTP request to a resolved server.
      * @param server IP address of the NTP server
      */
     void sendNtpRequest(const ip_addr_t* server);
 
     /**
      * @brief Retry the current request with the next server in the pool.
      */
     void retry();
 
     /**
      * @brief lwIP UDP receive callback for NTP replies.
      */
     static void ntpRecv(void* arg, struct udp_pcb* pcb, struct pbuf* p,
                         const ip_addr_t* addr, u16_t port);
 
     /**
      * @brief Process the NTP response packet and extract the epoch time.
      * @param p UDP payload buffer
      */
     void processResponse(struct pbuf* p);
 
     /**
      * @brief Set the system and RTC time based on epoch value.
      * @param epoch UNIX timestamp received from the NTP server
      */
     void setSystemTime(time_t epoch);
 };
 