#pragma once
#include <cstdio>
#include <ctime>

enum LogLevel {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

class Logger {
public:
    static void info(const char* msg)    { log(LOG_INFO, msg); }
    static void warn(const char* msg)    { log(LOG_WARN, msg); }
    static void error(const char* msg)   { log(LOG_ERROR, msg); }

    // Optional: change minimum log level (e.g., to suppress info)
    static void setMinLogLevel(LogLevel level) { minLevel = level; }

private:
    static LogLevel minLevel;

    static void log(LogLevel level, const char* msg) {
        if (level < minLevel) return;

        char timeBuf[20];
        getTimeString(timeBuf, sizeof(timeBuf));
        const char* levelStr = levelToString(level);

        printf("[%s] [%s] %s\n", timeBuf, levelStr, msg);

        // TODO: forward to SdFatStorageManager when implemented
        // if (storageReady) SdFatStorageManager::appendLog(...);
    }

    static void getTimeString(char* buffer, size_t len) {
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);
        strftime(buffer, len, "%Y-%m-%d %H:%M:%S", t);
    }

    static const char* levelToString(LogLevel level) {
        switch (level) {
            case LOG_INFO:  return "INFO";
            case LOG_WARN:  return "WARN";
            case LOG_ERROR: return "ERROR";
            default:        return "???";
        }
    }
};
