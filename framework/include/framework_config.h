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

// === Per-module enable flags ===
#define TRACE_NET     1
#define TRACE_HTTP    0
#define TRACE_AUTH    1
#define TRACE_APP     1

// === Global minimum log level (for all modules) ===
// Options: 0 = INFO, 1 = WARN, 2 = ERROR
#define TRACE_LEVEL_MIN TRACE_LVL_INFO

#endif // FRAMEWORK_CONFIG_H