/**
 * @file framework_config.h
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef FRAMEWORK_CONFIG_H
#define FRAMEWORK_CONFIG_H 

#pragma once

/**
 * @brief This setting defines the retry timeout 
 * The default SNTP retry time is 15 seconds, we set it to 5 in lwipopts.h
 * 
 * This setting defines how long we wait for SNTP sync
 * This is used by the TimeManager class
 *
 */
#ifndef NTP_TIMEOUT_SECONDS
#define NTP_TIMEOUT_SECONDS 15
#endif

#ifndef DETECT_LOCAL_TIMEZONE
#define DETECT_LOCAL_TIMEZONE 0 ///< Set to 1 to enable timezone detection, 0 to disable
#endif

#ifndef EVENT_QUEUE_LENGTH
#define EVENT_QUEUE_LENGTH 8 // Default max bufferred events in the queues
#endif

/**
 * @brief This setting defines the retry timeout for WiFi connection
 * The default is 15000 ms (15 seconds)
 */ 
#ifndef WIFI_RETRY_TIMEOUT_MS
#define WIFI_RETRY_TIMEOUT_MS     15000
#endif

/**
 * @brief This setting defines the maximum number of retries for WiFi connection
 */ 
#ifndef WIFI_MAX_RETRIES
#define WIFI_MAX_RETRIES          5
#endif

/**
 * @brief This setting defines whether to reboot the device on WiFi connection failure
 * The default is false, meaning the device will not reboot on failure
 * If set to true, the device will reboot after the maximum number of retries
 * This is used by the FrameworkManager class
 */
#ifndef WIFI_REBOOT_ON_FAILURE
#define WIFI_REBOOT_ON_FAILURE    false
#endif
/**
 * @brief This setting defines the interval for checking WiFi connection status
 * The default is 30000 ms (30 seconds)
 * This is used by the Network class to monitor WiFi connection status
 */
#ifndef WIFI_MONITOR_INTERVAL_MS
#define WIFI_MONITOR_INTERVAL_MS 30000  // Default: check every 30s
#endif



/**
 * @brief This setting defines where uploads go
 * The default is "/uploads"
 * This is used by the MultipartParser class
 */
#ifndef MULTIPART_UPLOAD_PATH
#define MULTIPART_UPLOAD_PATH "/uploads"
#endif

// Note that files of any length can be streamed from the server or uploaded multipart. 
// This is just the maximum size of the HTTP body that can be processed in one go.
// However if a regular request is made with a body larger than this, it will be truncated.

#if defined(PICO_RP2350)
#ifndef MAX_HTTP_BODY_LENGTH
#define MAX_HTTP_BODY_LENGTH 4 * 1024 ///< Maximum HTTP body size in bytes 
#endif
#else
#ifndef MAX_HTTP_BODY_LENGTH
#define MAX_HTTP_BODY_LENGTH 16 * 1024 ///< Maximum HTTP body size in bytes
#endif
#endif

#ifndef HTTP_IDLE_TIMEOUT
#define HTTP_IDLE_TIMEOUT 500 ///< Timeout for idle HTTP connections in milliseconds
#endif
#ifndef HTTP_RECEIVE_TIMEOUT
#define HTTP_RECEIVE_TIMEOUT 2000 ///< Timeout for receiving HTTP data in milliseconds
#endif

#ifndef HTTP_BUFFER_SIZE
#define HTTP_BUFFER_SIZE 1460 ///< Size of the HTTP buffer for request/response data
#endif

// === Framework configuration file ===
// This file contains various configuration settings for the framework.
#ifndef STREAM_SEND_DELAY_MS
#define STREAM_SEND_DELAY_MS 20
#endif

#ifndef ENABLE_GPIO_EVENTS
#define ENABLE_GPIO_EVENTS ///< Set to enable GPIO event handling, don't define to disable
#endif

// GPIO events can be configured to use either FreeRTOS notifications or event queues
// Notifications are faster and more efficient, but events allow for data passing and broadcasting).
// They can also be configured to do both
#ifndef GPIO_NOTIFICATIONS
#define GPIO_NOTIFICATIONS           1
#endif
#ifndef GPIO_EVENTS
#define GPIO_EVENTS                  2
#endif
#ifndef GPIO_EVENTS_AND_NOTIFICATIONS
#define GPIO_EVENTS_AND_NOTIFICATIONS (GPIO_NOTIFICATIONS | GPIO_EVENTS)
#endif

#ifndef GPIO_EVENT_HANDLING
#define GPIO_EVENT_HANDLING GPIO_EVENTS_AND_NOTIFICATIONS
#endif

// === Debug Trace Configuration ===

//#define QUIET_MODE ///< Set to disable all normal behavior print output, don't define to print

#ifdef QUIET_MODE
#ifndef QUIET_PRINTF
  #define QUIET_PRINTF(...) do {} while (0)
#endif
#else
#ifndef QUIET_PRINTF
  #define QUIET_PRINTF(...) printf(__VA_ARGS__)
#endif
#endif

// === Debug Trace Configuration ===

#ifndef TRACE_USE_TIMESTAMP
#define TRACE_USE_TIMESTAMP       1
#endif
#ifndef TRACE_USE_SD
#define TRACE_USE_SD              0   ///< Set to 1 to use SD card for trace output, 0 for UART
#endif

//#define LFS_TRACE_YES // set to enable LittleFS trace output, comment out to disable

// === Module-specific trace enables ===
#ifndef TRACE_AppContext
#define TRACE_AppContext          0
#endif
#ifndef TRACE_ChunkedDecoder
#define TRACE_ChunkedDecoder      0
#endif
#ifndef TRACE_FatFsStorageManager
#define TRACE_FatFsStorageManager 0
#endif
#ifndef TRACE_FrameworkApp
#define TRACE_FrameworkApp        0
#endif
#ifndef TRACE_FrameworkController
#define TRACE_FrameworkController 0
#endif
#ifndef TRACE_FrameworkModel
#define TRACE_FrameworkModel      0
#endif
#ifndef TRACE_FrameworkTask
#define TRACE_FrameworkTask       0
#endif
#ifndef TRACE_FrameworkManager
#define TRACE_FrameworkManager    0
#endif
#ifndef TRACE_HttpClient
#define TRACE_HttpClient          0
#endif
#ifndef TRACE_HttpFileserver
#define TRACE_HttpFileserver      0
#endif
#ifndef TRACE_HttpParser
#define TRACE_HttpParser          0
#endif
#ifndef TRACE_HttpRequest
#define TRACE_HttpRequest         0
#endif
#ifndef TRACE_HttpResponse
#define TRACE_HttpResponse        0
#endif
#ifndef TRACE_HttpServer
#define TRACE_HttpServer          0
#endif
#ifndef TRACE_Middleware
#define TRACE_Middleware          0
#endif
#ifndef TRACE_JsonParser
#define TRACE_JsonParser          0
#endif
#ifndef TRACE_JsonRequestHelper
#define TRACE_JsonRequestHelper   0
#endif
#ifndef TRACE_JsonService
#define TRACE_JsonService         0
#endif
#ifndef TRACE_JwtAuthenticator
#define TRACE_JwtAuthenticator    0
#endif
#ifndef TRACE_LittleFsStorageManager
#define TRACE_LittleFsStorageManager 0
#endif
#ifndef TRACE_LwipDnsResolver
#define TRACE_LwipDnsResolver     0
#endif
#ifndef TRACE_MultipartParser
#define TRACE_MultipartParser     0
#endif
#ifndef TRACE_Network
#define TRACE_Network             0
#endif
#ifndef TRACE_Router
#define TRACE_Router              0
#endif
#ifndef TRACE_Tcp
#define TRACE_Tcp                 0
#endif
#ifndef TRACE_utility
#define TRACE_utility             0  
#endif
#ifndef TRACE_TimeManager
#define TRACE_TimeManager         0
#endif
// etc.

// === Global minimum log level (for all modules) ===
// Options: 0 = INFO, 1 = WARN, 2 = ERROR
#ifndef TRACE_LEVEL_MIN
#define TRACE_LEVEL_MIN TRACE_LVL_INFO
#endif
#ifndef TRACE_LOG_PATH
#define TRACE_LOG_PATH "/framework_trace.log"
#endif

#endif // FRAMEWORK_CONFIG_H