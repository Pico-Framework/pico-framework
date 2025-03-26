#include "Logger.h"

LogLevel Logger::minLevel = LOG_INFO;

// examples

// Logger::info("Config loaded");
// Logger::warn("Voltage low");
// Logger::error("Failed to open file");
// Logger::setMinLogLevel(LOG_WARN);  // suppress INFO logs
