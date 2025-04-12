/**
 * @file Logger.cpp
 * @author Ian Archbell
 * @brief Implementation of Logger for structured logging to stdout and optionally SD.
 *
 * Supports logging with timestamps and severity levels (INFO, WARN, ERROR),
 * and can write logs to an SD card file via FatFsStorageManager.
 *
 * @version 0.2
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "Logger.h"
#include <cstring>
#include "AppContext.h"

LogLevel Logger::minLevel = LOG_INFO;

/// @copydoc Logger::info
void Logger::info(const char *msg)
{
    log(LOG_INFO, msg);
}

/// @copydoc Logger::warn
void Logger::warn(const char *msg)
{
    log(LOG_WARN, msg);
}

/// @copydoc Logger::error
void Logger::error(const char *msg)
{
    log(LOG_ERROR, msg);
}

/// @copydoc Logger::setMinLogLevel
void Logger::setMinLogLevel(LogLevel level)
{
    minLevel = level;
}

/// @copydoc Logger::enableFileLogging
void Logger::enableFileLogging(const std::string &path)
{
    logPath = path;
    logToFile = !logPath.empty();
}

/// @copydoc Logger::log
void Logger::log(LogLevel level, const char *msg)
{
    if (level < minLevel)
        return;

    char timeBuf[20];
    getTimeString(timeBuf, sizeof(timeBuf));
    const char *levelStr = levelToString(level);

    // Output to stdout
    printf("[%s] [%s] %s\n", timeBuf, levelStr, msg);

    // Optionally append to log file
    if (logToFile) {
        auto* storage = AppContext::getInstance().getService<StorageManager>();
        if (storage) {
            char fullMsg[256];
            snprintf(fullMsg, sizeof(fullMsg), "[%s] [%s] %s\n", timeBuf, levelStr, msg);
            storage->appendToFile(logPath, reinterpret_cast<const uint8_t*>(fullMsg), strlen(fullMsg));
        }
    }
    
}

/// @copydoc Logger::getTimeString
void Logger::getTimeString(char *buffer, size_t len)
{
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    strftime(buffer, len, "%Y-%m-%d %H:%M:%S", t);
}

/// @copydoc Logger::levelToString
const char *Logger::levelToString(LogLevel level)
{
    switch (level)
    {
    case LOG_INFO:
        return "INFO";
    case LOG_WARN:
        return "WARN";
    case LOG_ERROR:
        return "ERROR";
    default:
        return "???";
    }
}
