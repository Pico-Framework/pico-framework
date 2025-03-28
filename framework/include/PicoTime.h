#pragma once

#include <time.h>
#include <string>
#include "DS3231.h"
#include "hardware/rtc.h"

class PicoTime {
public:
    explicit PicoTime(DS3231& rtc);

    // Syncing system time and RTC
    bool setSystemTimeFromRTC();       // DS3231 → AON RTC
    bool setRTCFromSystemTime();       // AON RTC → DS3231

    void setSystemTime(time_t t);
    time_t getSystemTime() const;

    // Output
    void printNow();
    void printTime(time_t t);
    void print(const struct tm* t);
    void print(const datetime_t* dt);

    // Convenience
    struct tm nowTm() const;
    datetime_t nowDatetime();

    std::string getNowHhMmSs() const;

    static std::string todayHhMmSsString(const struct tm& hh_mm_ss);
    static struct tm todayHhMmSs(const struct tm* hh_mm_ss);

private:
    DS3231& rtc_;
};