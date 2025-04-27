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
 * @brief This setting defines where uploads go
 * The default is "/uploads"
 * This is used by the MultipartParser class
 */
#define MULTIPART_UPLOAD_PATH "/uploads"


// Note that files of any length can be streamed from the server or uploaded multipart. 
// This is just the maximum size of the HTTP body that can be processed in one go.
// However if a regular request is made with a body larger than this, it will be truncated.

#if defined(PICO_RP2350)
#define MAX_HTTP_BODY_LENGTH 4 * 1024 ///< Maximum HTTP body size in bytes 
#else
#define MAX_HTTP_BODY_LENGTH 16 * 1024 ///< Maximum HTTP body size in bytes
#endif

#define HTTP_BUFFER_SIZE 1460 ///< Size of the HTTP buffer for request/response data

// === Framework configuration file ===
// This file contains various configuration settings for the framework.
#define STREAM_SEND_DELAY_MS 20

#define ENABLE_GPIO_EVENTS ///< Set to enable GPIO event handling, don't define to disable

// GPIO events can be configured to use either FreeRTOS notifications or event queues
// Notifications are faster and more efficient, but events allow for data passing and broadcasting).
// They can also be configured to do both
#define GPIO_NOTIFICATIONS           1
#define GPIO_EVENTS                  2
#define GPIO_EVENTS_AND_NOTIFICATIONS (GPIO_NOTIFICATIONS | GPIO_EVENTS)

#ifndef GPIO_EVENT_HANDLING
#define GPIO_EVENT_HANDLING GPIO_EVENTS_AND_NOTIFICATIONS
#endif

// === Debug Trace Configuration ===

//#define QUIET_MODE ///< Set to disable all normal behavior print output, don't define to print

#ifdef QUIET_MODE
  #define QUIET_PRINTF(...) do {} while (0)
#else
  #define QUIET_PRINTF(...) printf(__VA_ARGS__)
#endif

// === Debug Trace Configuration ===

#define TRACE_USE_TIMESTAMP       1
#define TRACE_USE_SD              0   ///< Set to 1 to use SD card for trace output, 0 for UART

//#define LFS_TRACE_YES // set to enable LittleFS trace output, comment out to disable

// === Module-specific trace enables ===
#define TRACE_FrameworkManager    0
#define TRACE_HttpServer          0
#define TRACE_HttpRequest         0
#define TRACE_HttpResponse        0
#define TRACE_HttpFileserver      0
#define TRACE_MultipartParser     0
#define TRACE_JsonRequestHelper   0
#define TRACE_JwtAuthenticator    0
#define TRACE_Router              0
#define TRACE_FatFsStorageManager 0
#define TRACE_LittleFsStorageManager 1
#define TRACE_JsonService         0
#define TRACE_Middleware          0
#define TRACE_Network             0
#define TRACE_NtpClient           0
#define TRACE_utility             0  
#define TRACE_AppContext          0 
#define TRACE_Tcp                 0
#define TRACE_ChunkedDecoder      0
#define TRACE_JsonParser          0
#define TRACE_HttpClient          0
#define TRACE_LwipDnsResolver     0
#define TRACE_HttpParser          0
#define TRACE_TimeManager         0
// etc.

// === Global minimum log level (for all modules) ===
// Options: 0 = INFO, 1 = WARN, 2 = ERROR
#define TRACE_LEVEL_MIN TRACE_LVL_INFO
#define TRACE_LOG_PATH "/framework_trace.log"

#endif // FRAMEWORK_CONFIG_H