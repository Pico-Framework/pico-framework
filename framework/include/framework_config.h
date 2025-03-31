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


#define TRACE_USE_SD      1          ///< Set to 1 to use SD card for trace output, 0 for UART

// === Module-specific trace enables ===
#define TRACE_FrameworkManager    1
#define TRACE_HttpServer          1
#define TRACE_HttpRequest         1
#define TRACE_HttpResponse        1
#define TRACE_HttpFileserver      1
#define TRACE_MultipartParser     1
#define TRACE_JsonRequestHelper   1
#define TRACE_JwtAuthenticator    1
#define TRACE_Router              1
#define TRACE_FatFsStorageManager 1
#define TRACE_JsonService         1
#define TRACE_Middileware         1
#define TRACE_Network             1
#define TRACE_NtpClient           1
// etc.

// === Global minimum log level (for all modules) ===
// Options: 0 = INFO, 1 = WARN, 2 = ERROR
#define TRACE_LEVEL_MIN TRACE_LVL_INFO
#define TRACE_LOG_PATH "/framework_trace.log"

#endif // FRAMEWORK_CONFIG_H