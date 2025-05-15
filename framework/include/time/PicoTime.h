/**
 * @file PicoTime.h
 * @author Ian Archbell
 * @brief Time utility functions for the Pico platform (RP2040/RP2350).
 *
 * Part of the PicoFramework application framework.
 * Provides real-time clock (RTC/AON) access and helpers for time conversion,
 * formatting, and output. Designed for use in embedded systems where standard
 * time libraries are limited or unavailable.
 *
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#pragma once

#include <ctime>
#include <string>
#include "pico/stdlib.h"

#include "time/TimeOfDay.h"
#include "time/DaysOfWeek.h"

#if PICO_RP2040
#include "hardware/rtc.h"
#endif

/**
 * @class PicoTime
 * @brief Cross-platform time utilities for RP2040 and RP2350.
 */
class PicoTime
{
public:
    /**
     * @brief Get the current epoch time using platform RTC or AON.
     * @return Current UNIX timestamp.
     */
    static time_t now();

    /**
     * @brief Get the current local time as `struct tm`.
     * @return Current time in `tm` format.
     */
    static struct tm nowTm();

    /**
     * @brief Get the current time as a `datetime_t`.
     * @return Time in `datetime_t` format.
     */
    #if PICO_RP2040
    static datetime_t nowDatetime();
    #endif
    /**
     * @brief Get a string with the current time formatted as HH:MM:SS.
     * @return Formatted time string.
     */
    static std::string getNowHhMmSs();

    /**
     * @brief Construct today's date with a specific time-of-day.
     * @param hhmmss Pointer to a `tm` with hour/min/sec set.
     * @return Combined `tm` with today's date and given time.
     */
    static struct tm todayAt(const struct tm *hhmmss);

    /**
     * @brief Convert a `tm` with today's date and time to epoch.
     * @param hhmmss Pointer to `tm` with time-of-day.
     * @return Time as UNIX timestamp.
     */
    static time_t todayAtTimeT(const struct tm *hhmmss);

    /**
     * @brief Convert a time-of-day to a string for today.
     * @param hhmmss Pointer to `tm` with hour/min/sec.
     * @return Formatted string including date and time.
     */
    static std::string todayHhMmSsString(const struct tm *hhmmss);

    /**
     * @brief Print the current time to stdout.
     */
    static void printNow();

    /**
     * @brief Print a given UNIX timestamp to stdout.
     * @param t UNIX time value.
     */
    static void print(time_t t);

    /**
     * @brief Print a `struct tm` to stdout.
     * @param t Pointer to `tm` to print.
     */
    static void print(const struct tm *t);

    /**
     * @brief Print a `datetime_t` to stdout.
     * @param dt Pointer to `datetime_t` to print.
     */
    #if PICO_RP2040

    static void print(const datetime_t *dt);
    #endif
    
    static TimeOfDay toTimeOfDay(uint32_t timestamp) {
        time_t t = static_cast<time_t>(timestamp);
        struct tm tm;
        localtime_r(&t, &tm);
        return TimeOfDay{ static_cast<uint8_t>(tm.tm_hour), static_cast<uint8_t>(tm.tm_min), static_cast<uint8_t>(tm.tm_sec) };
    }
    
    static DaysOfWeek dayOfWeekBitmask(uint32_t timestamp) {
        uint32_t days = timestamp / 86400;
        uint8_t weekday = (days + 4) % 7;  // Sunday = 0
        return static_cast<DaysOfWeek>(1u << weekday);
    }  
    
    static Day dayOfWeek(uint32_t timestamp) {
        uint32_t days = timestamp / 86400;
        uint8_t weekday = (days + 4) % 7;  // Sunday = 0
        return static_cast<Day>(1u << weekday);
    }

    static std::string formatIso8601(time_t t);  // e.g., "2025-05-15T06:00:00"

};
