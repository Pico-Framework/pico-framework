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
 
//  #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
 
//  /**
//   * @brief Debug trace macro with optional file and line info.
//   */
//  #ifdef TRACE_ON
//      #define TRACE(format, ...) printf("TRACE: %s:%d: " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)
//  #else
//      #define TRACE(format, ...) /* no-op */
//  #endif
 
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
 
 #endif // UTILITY_H
 