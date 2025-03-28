#include "PicoTime.h"
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iomanip>

PicoTime::PicoTime(DS3231& rtc) : rtc_(rtc) {}

bool PicoTime::setSystemTimeFromRTC() {
    time_t t = rtc_.getTimestamp();
    setSystemTime(t);
    return true;
}

bool PicoTime::setRTCFromSystemTime() {
    time_t t = getSystemTime();
    rtc_.setTimestamp(t);
    return true;
}

void PicoTime::setSystemTime(time_t t) {
    struct tm tm;
    gmtime_r(&t, &tm);
    datetime_t dt = {
        .year = (int16_t)(tm.tm_year + 1900),
        .month = (int8_t)(tm.tm_mon + 1),
        .day = (int8_t)(tm.tm_mday),
        .dotw = 0,
        .hour = (int8_t)(tm.tm_hour),
        .min = (int8_t)(tm.tm_min),
        .sec = (int8_t)(tm.tm_sec)
    };
    rtc_set_datetime(&dt);
}

time_t PicoTime::getSystemTime() const{
    datetime_t dt;
    if (!rtc_get_datetime(&dt)) return 0;
    struct tm t = {};
    t.tm_year = dt.year - 1900;
    t.tm_mon  = dt.month - 1;
    t.tm_mday = dt.day;
    t.tm_hour = dt.hour;
    t.tm_min  = dt.min;
    t.tm_sec  = dt.sec;
    return mktime(&t);
}

void PicoTime::printNow() {
    printTime(getSystemTime());
}

void PicoTime::printTime(time_t t) {
    struct tm tm;
    localtime_r(&t, &tm);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    printf("%s\n", buf);
}

void PicoTime::print(const struct tm* t) {
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
    printf("%s\n", buf);
}

void PicoTime::print(const datetime_t* dt) {
    printf("%04d-%02d-%02d %02d:%02d:%02d\n",
           dt->year, dt->month, dt->day,
           dt->hour, dt->min, dt->sec);
}

struct tm PicoTime::nowTm() const {
    time_t now = getSystemTime();
    struct tm t;
    localtime_r(&now, &t);
    return t;
}

datetime_t PicoTime::nowDatetime() {
    datetime_t dt;
    rtc_get_datetime(&dt);
    return dt;
}

std::string PicoTime::getNowHhMmSs() const {
    struct tm now = nowTm();
    char buf[9];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", now.tm_hour, now.tm_min, now.tm_sec);
    return std::string(buf);
}

std::string PicoTime::todayHhMmSsString(const struct tm& hh_mm_ss) {
    struct tm t = todayHhMmSs(&hh_mm_ss);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
    return std::string(buf);
}

struct tm PicoTime::todayHhMmSs(const struct tm* hh_mm_ss) {
    time_t now_t = time(NULL);
    struct tm now;
    localtime_r(&now_t, &now);

    struct tm result = {
        .tm_sec  = hh_mm_ss->tm_sec,
        .tm_min  = hh_mm_ss->tm_min,
        .tm_hour = hh_mm_ss->tm_hour,
        .tm_mday = now.tm_mday,
        .tm_mon  = now.tm_mon,
        .tm_year = now.tm_year,
        .tm_isdst = -1
    };
    mktime(&result); // Normalize
    return result;
}