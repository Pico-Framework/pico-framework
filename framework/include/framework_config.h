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

// === Framework configuration file ===
// This file contains various configuration settings for the framework.
#define STREAM_SEND_DELAY_MS 20

// === Debug Trace Configuration ===

#define TRACE_USE_TIMESTAMP       1
#define TRACE_USE_SD              0   ///< Set to 1 to use SD card for trace output, 0 for UART

// === Module-specific trace enables ===
#define TRACE_FrameworkManager    0
#define TRACE_HttpServer          1
#define TRACE_HttpRequest         0
#define TRACE_HttpResponse        0
#define TRACE_HttpFileserver      0
#define TRACE_MultipartParser     0
#define TRACE_JsonRequestHelper   0
#define TRACE_JwtAuthenticator    0
#define TRACE_Router              0
#define TRACE_FatFsStorageManager 0
#define TRACE_JsonService         0
#define TRACE_Middileware         0
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