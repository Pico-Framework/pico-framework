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

#define TRACE_ENABLED     1               ///< Set to 0 to disable all tracing
#define TRACE_USE_SD      1          ///< Set to 1 to use SD card for trace output, 0 for UART

// === Global minimum log level (for all modules) ===
// Options: 0 = INFO, 1 = WARN, 2 = ERROR
#define TRACE_LEVEL_MIN TRACE_LVL_INFO

#endif // FRAMEWORK_CONFIG_H