#pragma once
#include <cstdio>
#include <ctime>
#include <cstring>
#include <cstdarg>
#include "trace_config.h"

// === Log levels ===
enum TraceLevel {
    TRACE_LVL_INFO  = 0,
    TRACE_LVL_WARN  = 1,
    TRACE_LVL_ERROR = 2
};

class DebugTrace {
public:
    static void print(TraceLevel level, const char* module, const char* func, const char* file, int line, const char* msg) {
        char timeBuf[9];
        getTimeString(timeBuf, sizeof(timeBuf));
        const char* shortFile = shortenFilePath(file);
        const char* levelStr = levelToString(level);

        ::printf("[%s] [%s] [%s::%s@%d] %s\n", timeBuf, levelStr, module, shortFile, line, msg);
    }

    static void printf(TraceLevel level, const char* module, const char* func, const char* file, int line, const char* fmt, va_list args) {
        char timeBuf[9];
        getTimeString(timeBuf, sizeof(timeBuf));
        const char* shortFile = shortenFilePath(file);
        const char* levelStr = levelToString(level);

        char msgBuf[256];
        vsnprintf(msgBuf, sizeof(msgBuf), fmt, args);

        ::printf("[%s] [%s] [%s::%s@%d] %s\n", timeBuf, levelStr, module, shortFile, line, msgBuf);
    }

private:
    static void getTimeString(char* buffer, size_t len) {
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);
        strftime(buffer, len, "%H:%M:%S", t);
    }

    static const char* shortenFilePath(const char* path) {
        const char* slash = strrchr(path, '/');
#ifdef _WIN32
        const char* backslash = strrchr(path, '\\');
        if (backslash && (!slash || backslash > slash)) slash = backslash;
#endif
        return slash ? slash + 1 : path;
    }

    static const char* levelToString(TraceLevel level) {
        switch (level) {
            case TRACE_LVL_INFO: return "INFO";
            case TRACE_LVL_WARN: return "WARN";
            case TRACE_LVL_ERROR: return "ERROR";
            default: return "???";
        }
    }
};

// === Internal macros ===
#define TRACE_ENABLED(MODULE)  (TRACE_##MODULE)
#define TRACE_LEVEL_ALLOWED(LEVEL) ((LEVEL) >= TRACE_LEVEL_MIN)

// === Public trace macro initializer ===
#define TRACE_INIT(MODULE)                                              \
    constexpr const char* __trace_module_name__ = #MODULE;             \
    constexpr bool __trace_enabled__ = TRACE_ENABLED(MODULE);          \
                                                                        \
    inline void TRACE_INFO(const char* msg) {                           \
        if (__trace_enabled__ && TRACE_LEVEL_ALLOWED(TRACE_LVL_INFO))  \
            DebugTrace::print(TRACE_LVL_INFO, __trace_module_name__, __func__, __FILE__, __LINE__, msg); \
    }                                                                  \
    inline void TRACE_WARN(const char* msg) {                           \
        if (__trace_enabled__ && TRACE_LEVEL_ALLOWED(TRACE_LVL_WARN))  \
            DebugTrace::print(TRACE_LVL_WARN, __trace_module_name__, __func__, __FILE__, __LINE__, msg); \
    }                                                                  \
    inline void TRACE_ERROR(const char* msg) {                          \
        if (__trace_enabled__ && TRACE_LEVEL_ALLOWED(TRACE_LVL_ERROR)) \
            DebugTrace::print(TRACE_LVL_ERROR, __trace_module_name__, __func__, __FILE__, __LINE__, msg); \
    }                                                                  \
    inline void TRACEF_INFO(const char* fmt, ...) {                     \
        if (__trace_enabled__ && TRACE_LEVEL_ALLOWED(TRACE_LVL_INFO)) {\
            va_list args;                                              \
            va_start(args, fmt);                                       \
            DebugTrace::printf(TRACE_LVL_INFO, __trace_module_name__, __func__, __FILE__, __LINE__, fmt, args); \
            va_end(args);                                              \
        }                                                              \
    }                                                                  \
    inline void TRACEF_WARN(const char* fmt, ...) {                     \
        if (__trace_enabled__ && TRACE_LEVEL_ALLOWED(TRACE_LVL_WARN)) {\
            va_list args;                                              \
            va_start(args, fmt);                                       \
            DebugTrace::printf(TRACE_LVL_WARN, __trace_module_name__, __func__, __FILE__, __LINE__, fmt, args); \
            va_end(args);                                              \
        }                                                              \
    }                                                                  \
    inline void TRACEF_ERROR(const char* fmt, ...) {                    \
        if (__trace_enabled__ && TRACE_LEVEL_ALLOWED(TRACE_LVL_ERROR)) {\
            va_list args;                                              \
            va_start(args, fmt);                                       \
            DebugTrace::printf(TRACE_LVL_ERROR, __trace_module_name__, __func__, __FILE__, __LINE__, fmt, args); \
            va_end(args);                                              \
        }                                                              \
    }
