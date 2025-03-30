#pragma once

#include <time.h>
#include <string>
//#include "DS3231.h" support coming soon
#include "hardware/rtc.h"

#pragma once

#include <ctime>
#include <string>
#include "pico/stdlib.h"

#if PICO_ON_CHIP_RTC
#include "hardware/rtc.h"
#endif

class PicoTime {
public:
    // Platform-aware time fetch
    static time_t now();
    static struct tm nowTm();
    static datetime_t nowDatetime();

    // Time-of-day helpers
    static std::string getNowHhMmSs();
    static struct tm todayAt(const struct tm* hhmmss);
    static time_t todayAtTimeT(const struct tm* hhmmss);
    static std::string todayHhMmSsString(const struct tm* hhmmss);

    // Output
    static void printNow();
    static void print(time_t t);
    static void print(const struct tm* t);
    static void print(const datetime_t* dt);
};