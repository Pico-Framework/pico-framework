#pragma once

#include <cstdint>
#include <string>
#include <cstdio>

#include "nlohmann/json.hpp"
/**
 * @file TimeOfDay.h
 * @author Ian Archbell
 * @brief A simple representation of wall-clock time (HH:MM or HH:MM:SS)
 * @version 1.0
 * @date 2025-05-10
 * @license MIT
 * @copyright Copyright (c) 2025, Ian Archbell
 */

/**
 * @brief A simple value type representing a time of day (hour, minute, second).
 */
struct TimeOfDay
{
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;

    /**
     * @brief Parse a string in the form "HH:MM" or "HH:MM:SS".
     * @param hhmm Input time string.
     * @return Parsed TimeOfDay (defaults to 00:00:00 on error).
     */
    static TimeOfDay fromString(const char *hhmm) {
        TimeOfDay t;
        if (sscanf(hhmm, "%2hhu:%2hhu:%2hhu", &t.hour, &t.minute, &t.second) == 3) {
            return t;
        } else if (sscanf(hhmm, "%2hhu:%2hhu", &t.hour, &t.minute) == 2) {
            t.second = 0;
            return t;
        }
        return {0, 0, 0};
    }

    /**
     * @brief Format the time as a string.
     * @param time Input time.
     * @return Formatted string, either "HH:MM" or "HH:MM:SS" depending on seconds.
     */
    static std::string toString(const TimeOfDay &time) {
        char buf[9];
        if (time.second > 0) {
            snprintf(buf, sizeof(buf), "%02u:%02u:%02u", time.hour, time.minute, time.second);
        } else {
            snprintf(buf, sizeof(buf), "%02u:%02u", time.hour, time.minute);
        }
        return std::string(buf);
    }

    // Comparison operators

    bool operator==(const TimeOfDay &other) const {
        return hour == other.hour && minute == other.minute && second == other.second;
    }

    bool operator!=(const TimeOfDay &other) const {
        return !(*this == other);
    }

    bool operator<(const TimeOfDay &other) const {
        if (hour != other.hour) return hour < other.hour;
        if (minute != other.minute) return minute < other.minute;
        return second < other.second;
    }

    bool operator<=(const TimeOfDay &other) const {
        return !(*this > other);
    }

    bool operator>(const TimeOfDay &other) const {
        return other < *this;
    }

    bool operator>=(const TimeOfDay &other) const {
        return !(*this < other);
    }
    
};
