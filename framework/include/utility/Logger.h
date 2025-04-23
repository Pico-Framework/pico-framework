/**
 * @file Logger.h
 * @author Ian Archbell
 * @brief Lightweight structured logging class for embedded applications.
 *
 * Part of the PicoFramework application framework.
 * Logs messages with timestamps and severity levels (INFO, WARN, ERROR).
 * Optionally supports logging to an SD card via FatFsStorageManager.
 * 
 * Usage example:
 * @code
 * Logger::setMinLogLevel(LOG_INFO);  // Set minimum log level to INFO
 * Logger::enableFileLogging(AppContext::storage(), "/log/system.log");
 * Logger::info("System started successfully.");
 * Logger::warn("Low battery warning.");    
 * Logger::error("Critical error occurred.");
 * @endcode
 *
 * @version 0.2
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #ifndef LOGGER_H
 #define LOGGER_H
 #pragma once
 
 #include <cstdio>
 #include <ctime>
 #include <string>
 #include "storage/StorageManager.h"
 
 /**
  * @brief Severity levels for logging.
  */
 enum LogLevel {
     LOG_INFO,   ///< Informational messages
     LOG_WARN,   ///< Warnings (non-fatal)
     LOG_ERROR   ///< Errors (potentially requiring user action)
 };
 
 /**
  * @class Logger
  * @brief Basic timestamped logger with optional SD file logging.
  *
  * This logger is designed for embedded use, providing filtered logging
  * based on severity and timestamp, and supporting optional output to SD.
  */
 class Logger {
 public:
     /**
      * @brief Log an informational message.
      * @param msg The message to log.
      */
     static void info(const char* msg);
 
     /**
      * @brief Log a warning message.
      * @param msg The message to log.
      */
     static void warn(const char* msg);
 
     /**
      * @brief Log an error message.
      * @param msg The message to log.
      */
     static void error(const char* msg);
 
     /**
      * @brief Set the minimum log level (filters lower levels).
      * @param level Messages below this level will be suppressed.
      */
     static void setMinLogLevel(LogLevel level);
 
     /**
      * @brief Enable writing logs to SD card via a storage manager.
      * @param path Path to the log file on SD card (e.g. "/log/system.log").
      */
     static void enableFileLogging(const std::string& path);
 
 private:
     static LogLevel minLevel;
     static void log(LogLevel level, const char* msg);
     static void getTimeString(char* buffer, size_t len);
     static const char* levelToString(LogLevel level);
 
     static inline std::string logPath = "";
     static inline bool logToFile = false;
 };
 
 #endif // LOGGER_H
 