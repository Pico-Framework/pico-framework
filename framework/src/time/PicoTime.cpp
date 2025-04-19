/**
 * @file PicoTime.cpp
 * @author Ian Archbell
 * @brief Implementation of time utilities for RP2040 and RP2350 platforms.
 *
 * Uses RTC on RP2040 and AON timer on RP2350 to fetch and format time data.
 * Also provides formatted string output and time conversion helpers.
 *
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "PicoTime.h"
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iomanip>

#if defined(PICO_RP2040)
#include "hardware/rtc.h"
#elif defined(PICO_RP2350)
#include "pico/aon_timer.h"
#endif

/// @copydoc PicoTime::now
time_t PicoTime::now()
{
#if defined(PICO_RP2040)
    datetime_t dt;
    rtc_get_datetime(&dt);
    struct tm t = {};
    t.tm_year = dt.year - 1900;
    t.tm_mon = dt.month - 1;
    t.tm_mday = dt.day;
    t.tm_hour = dt.hour;
    t.tm_min = dt.min;
    t.tm_sec = dt.sec;
    return mktime(&t);
#elif defined(PICO_RP2350)
    // Use AON timer for RP2350
    timespec t;
    aon_timer_get_time(&t);
    return t.tv_sec; // Return seconds since epoch
#endif    
}    

/// @copydoc PicoTime::nowTm
struct tm PicoTime::nowTm()
{
    time_t t = now();
    struct tm out;
    localtime_r(&t, &out);
    return out;
}

/// @copydoc PicoTime::nowDatetime
#if defined(PICO_RP2040)
datetime_t PicoTime::nowDatetime()
{
    datetime_t dt;
    rtc_get_datetime(&dt);
    return dt;  
}
#endif

/// @copydoc PicoTime::getNowHhMmSs
std::string PicoTime::getNowHhMmSs()
{
    struct tm t = nowTm();
    char buf[9];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
    return std::string(buf);
}

/// @copydoc PicoTime::todayAt
struct tm PicoTime::todayAt(const struct tm *hhmmss)
{
    struct tm now = nowTm();
    struct tm result = {
        .tm_sec = hhmmss->tm_sec,
        .tm_min = hhmmss->tm_min,
        .tm_hour = hhmmss->tm_hour,
        .tm_mday = now.tm_mday,
        .tm_mon = now.tm_mon,
        .tm_year = now.tm_year,
        .tm_isdst = -1};
    mktime(&result); // Normalize
    return result;
}

/// @copydoc PicoTime::todayAtTimeT
time_t PicoTime::todayAtTimeT(const struct tm *hhmmss)
{
    struct tm t = todayAt(hhmmss);
    return mktime(&t);
}

/// @copydoc PicoTime::todayHhMmSsString
std::string PicoTime::todayHhMmSsString(const struct tm *hhmmss)
{
    struct tm t = todayAt(hhmmss);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
    return std::string(buf);
}

/// @copydoc PicoTime::printNow
void PicoTime::printNow()
{
    print(now());
}

/// @copydoc PicoTime::print(time_t)
void PicoTime::print(time_t t)
{
    struct tm tm;
    localtime_r(&t, &tm);
    print(&tm);
}

/// @copydoc PicoTime::print(const struct tm*)
void PicoTime::print(const struct tm *t)
{
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
    printf("%s\n", buf);
}

/// @copydoc PicoTime::print(const datetime_t*)
#if defined(PICO_RP2040)
void PicoTime::print(const datetime_t *dt)
{
    printf("%04d-%02d-%02d %02d:%02d:%02d\n",
           dt->year, dt->month, dt->day,
           dt->hour, dt->min, dt->sec);
}
#endif
