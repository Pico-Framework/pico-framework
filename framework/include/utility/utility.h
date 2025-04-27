/**
 * @file utility.h
 * @author Ian Archbell
 * @brief System utilities for diagnostics, memory, stack usage, and tracing.
 *
 * Provides functions to report heap, stack, and lwIP memory stats,
 * along with runtime diagnostics. Intended for use in embedded applications.
 *
 * @version 0.1
 * @date 2025-03-26
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#ifndef UTILITY_H
#define UTILITY_H
#pragma once

#include <string>
#include <cstring>
#include <unordered_map>

/// @brief Print stack high watermark for all FreeRTOS tasks.
void printTaskStackSizes();

/// @brief Print heap size and minimum ever free heap.
void printHeapInfo();

/// @brief Print stack and heap usage together.
void printSystemMemoryInfo();

/// @brief Convert string to lowercase.
std::string toLower(std::string str);

/// @brief Display task priorities, stack usage, and heap stats.
void runTimeStats();

/// @brief Print number of active lwIP TCP PCBs.
void printActivePCBs();

/// @brief Print FreeRTOS heap and task stack status.
void logSystemStats();

/// @brief Print full lwIP statistics if available.
void printTCPState();

/// @brief Print lwIP memory pool statistics.
void printMemsize();

/// @brief Check whether currently in interrupt context.
int is_in_interrupt(void);

/// @brief Print in a mode that can be used in an ISR context.
void debug_print(const char *msg);
void debug_print(const std::string &msg);
void debug_print(const char *msg, size_t len);

#endif // UTILITY_H
