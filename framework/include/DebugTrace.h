/**
 * @file DebugTrace.h
 * @author Ian Archbell
 * @brief Macro-based debug trace system with optional SD file logging.
 *
 * Part of the PicoFramework application framework.
 * Provides compile-time enabled, level-filtered trace logging for embedded applications.
 * Features:
 * - Per-module control (`TRACE_<MODULE>` in framework_config.h)
 * - Level filtering (`TRACE_LEVEL_MIN`)
 * - Optional timestamp
 * - Optional SD file redirection via FatFsStorageManager
 *
 * Controlled via `framework_config.h`. SD output is enabled if `TRACE_USE_SD` is set to 1.
 *
 * @version 0.3
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #pragma once

 #include <cstdio>
 #include <cstdarg>
 #include <ctime>
 #include <string>
 #include <cstring>
 #include "framework_config.h"
 #include "StorageManager.h"
 #include "AppContext.h"
 #include "TimeManager.h"
 
 #define TRACE_LVL_INFO 0
 #define TRACE_LVL_WARN 1
 #define TRACE_LVL_ERROR 2
 
 #ifndef TRACE_INCLUDE_TIMESTAMP
 #define TRACE_INCLUDE_TIMESTAMP 1
 #endif
 
 // File trace output (set by framework setup)
 static inline std::string tracePath;
 static inline bool traceToFile = false;
 
 /**
  * @brief Set trace output to SD file using FatFsStorageManager.
  * @param sm Storage manager (or nullptr for console only)
  * @param path File path (e.g. "/log/trace.txt")
  */
 inline void setTraceOutputToFile(StorageManager *sm, const std::string &path)
 {
     tracePath = path;
     traceToFile = (sm != nullptr && !path.empty());
 }
 
 /**
  * @brief Convert trace level to string.
  */
 inline const char *traceLevelToString(int level)
 {
     switch (level)
     {
     case TRACE_LVL_INFO:
         return "INFO";
     case TRACE_LVL_WARN:
         return "WARN";
     case TRACE_LVL_ERROR:
         return "ERROR";
     default:
         return "???";
     }
 }
 
 /**
  * @brief Strip file path to filename only.
  */
 inline void shortenFilePath(const char *fullPath, char *buffer, size_t maxLen)
 {
     const char *slash = strrchr(fullPath, '/');
     snprintf(buffer, maxLen, "%s", slash ? slash + 1 : fullPath);
 }
 
 enum TraceTimeFormat
 {
     TRACE_TIME_UTC,
     TRACE_TIME_LOCAL
 };
 static TraceTimeFormat traceTimeFormat = TRACE_TIME_LOCAL;
 
 /**
  * @brief Internal trace log handler. Formats and dispatches output.
  */
 
 inline std::string getFormattedTimestamp()
 {
     static bool timeAvailable = false;
     static const TimeManager *tm = nullptr;
 
     if (!timeAvailable)
     {
         tm = AppContext::getInstance().getService<TimeManager>();
         timeAvailable = tm != nullptr;
     }
 
     if (tm)
     {
         return tm->currentTimeForTrace();
     }
     else
     {
 
         return std::string("");
     }
 }
 
 inline void traceLog(const char *module, int level, const char *file, int line, const char *func, const char *format, ...)
 {
     if (level < TRACE_LEVEL_MIN)
         return;
 
     char fileBuf[64];
     shortenFilePath(file, fileBuf, sizeof(fileBuf));
 
     char msgBuf[256];
     va_list args;
     va_start(args, format);
     vsnprintf(msgBuf, sizeof(msgBuf), format, args);
     va_end(args);
     std::string formatted;
     std::string timestamp = "";
 
 #if TRACE_INCLUDE_TIMESTAMP
     timestamp = getFormattedTimestamp();
 
     formatted = timestamp + "[" + traceLevelToString(level) + "] [" +
                 module + "] " + fileBuf + ":" + std::to_string(line) +
                 " (" + func + "): " + msgBuf + "\n";
 
 #endif
     if (traceToFile)
     {
         auto *storage = AppContext::getInstance().getService<StorageManager>();
         if (storage)
         {
             storage->appendToFile(tracePath, reinterpret_cast<const uint8_t *>(formatted.c_str()), formatted.length());
         }
     }
     else
     {
         fputs(formatted.c_str(), stdout);
         fflush(stdout);
     }
 }
 
 /**
  * @brief Declare trace usage in a source file for a given module.
  */
 #define TRACE_INIT(MODULE_NAME)                               \
     static constexpr const char *TRACE_MODULE = #MODULE_NAME; \
     static constexpr int TRACE_ENABLED = TRACE_##MODULE_NAME;
 
 /**
  * @brief Core trace macro with level support.
  */
 #define TRACEF(level, ...)                                                            \
     do                                                                                \
     {                                                                                 \
         if (TRACE_ENABLED && (level) >= TRACE_LEVEL_MIN)                              \
             traceLog(TRACE_MODULE, level, __FILE__, __LINE__, __func__, __VA_ARGS__); \
     } while (0)
 
 /**
  * @brief Default trace (INFO level).
  */
 #define TRACE(...) TRACEF(TRACE_LVL_INFO, __VA_ARGS__)
 
 /**
  * @brief WARN level trace.
  */
 #define TRACE_WARN(...) TRACEF(TRACE_LVL_WARN, __VA_ARGS__)
 
 /**
  * @brief ERROR level trace.
  */
 #define TRACE_ERROR(...) TRACEF(TRACE_LVL_ERROR, __VA_ARGS__)
 