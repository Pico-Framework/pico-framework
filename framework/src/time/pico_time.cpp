#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <time.h>
#include "hardware/rtc.h"


#include "pico_time.hpp"

/**
 * @brief Singleton Instance
 */
PicoTime& PicoTime::getInstance() {
    static PicoTime instance;  // No need for `new`
    return instance;
}

/**
 * @brief Sets the time zone
 */
void PicoTime::setTimeZone(const std::string& tz) {
    _time_zone = tz;
    setenv("TZ", _time_zone.c_str(), true);
    tzset();
}

/**
 * @brief Set the system time
 */
bool PicoTime::setSystemTime(time_t t) {
    time_t safe_result = t;
    printf("setSystemTime() called with: %ld (expected: > 1609459200)\n", static_cast<long long>(safe_result));
    
    if (safe_result < 1609459200) {  // Check if the time is being corrupted before setting
        printf("Error: Invalid time_t value passed to setSystemTime: %ld\n", t);
        return false;
    }

    struct timeval now = { .tv_sec = safe_result, .tv_usec = 0 };
    int ret = settimeofday(&now, NULL);

    if (ret == 0) {
        printf("System time successfully updated to: %ld\n", time(NULL));
        return true;
    } else {
        perror("settimeofday failed");
        return false;
    }
}


bool PicoTime::setSystemTime(ds3231_data_t* dt) {
    return setSystemTime(ds3231_data_to_time(dt));
}

/**
 * @brief Get the system time
 */
time_t PicoTime::getSystemTime() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec;
}

/**
 * @brief Get the Now Hh Mm Ss object
 * 
 * @return std::string 
 */
std::string PicoTime::getNowHhMmSs() {
    time_t now = time(0);
    struct tm *utc = gmtime(&now);
    char buff[sizeof("hh:mm:ss")];
    strftime(buff, sizeof(buff), "%H:%M:%S", utc);
    return buff;
}

/**
 * @brief 
 * 
 * @param hh_mm_ss 
 * @return std::string 
 */
std::string PicoTime::todayHhMmSsString(tm hh_mm_ss) {
    // timers need to be up for this
    time_t now = time(0);
    struct tm *utc = gmtime(&now);
    utc->tm_hour = hh_mm_ss.tm_hour;
    utc->tm_min = hh_mm_ss.tm_min;
    utc->tm_sec = hh_mm_ss.tm_sec;
    char buff[std::size("yyyy-mm-ddThh:mm:ssZ")];
    strftime(buff, sizeof(buff), "%FT%TZ", utc);
    return buff;
}

/**
 * @brief 
 * 
 * @param hh_mm_ss 
 * @return tm 
 */
tm PicoTime::todayHhMmSs(tm* hh_mm_ss) {
    time_t now = time(0);
    struct tm *utc = gmtime(&now);
    utc->tm_hour = hh_mm_ss->tm_hour;
    utc->tm_min = hh_mm_ss->tm_min;
    utc->tm_sec = hh_mm_ss->tm_sec;
    return *utc;
}

/**
 * @brief Converts DS3231 data to time_t
 */
time_t PicoTime::ds3231_data_to_time(const ds3231_data_t* t) {
    struct tm timeinfo = {};
    timeinfo.tm_year = t->year + 100;  // DS3231 year starts at 2000
    timeinfo.tm_mon = t->month - 1;
    timeinfo.tm_mday = t->date;
    timeinfo.tm_hour = t->hours;
    timeinfo.tm_min = t->minutes;
    timeinfo.tm_sec = t->seconds;
    return mktime(&timeinfo);
}

/**
 * @brief Converts struct tm (NTP format) to DS3231 data format
 */
ds3231_data_t PicoTime::tm_to_ds3231_data(const tm* tm){
    ds3231_data_t ds3231_data;
    ds3231_data.am_pm = false;
    ds3231_data.century = 1;
    ds3231_data.year = tm->tm_year; // need to check starting year
    ds3231_data.month = tm->tm_mon; // may need to add 1
    ds3231_data.date = tm->tm_mday;
    ds3231_data.day = tm->tm_wday;
    ds3231_data.hours = tm->tm_hour;
    ds3231_data.minutes = tm->tm_min;
    ds3231_data.seconds = tm->tm_sec;
    return ds3231_data;
}

/**
 * @brief Print the current system time
 */
void PicoTime::printSystemTime() {
    struct timeval tv;  // Timeval struct       
    gettimeofday(&tv, NULL);  // Get the current time
    time_t t = tv.tv_sec;  // Get the seconds
    // convert to printable format
    printf("System time: %s", asctime(gmtime(&t)));
}

/**
 * @brief Print time from different formats
 */
void PicoTime::print(const time_t* t) {
    printf("%s", asctime(gmtime(t)));
}

void PicoTime::print(const tm* t) {
    printf("%s", asctime(t));
}

void PicoTime::print(const ds3231_data_t* t) {
    time_t ct = ds3231_data_to_time(t);
    print(&ct);
}

/**
 * @brief Default Timezone
 */
std::string PicoTime::_time_zone = "GMT"; // Default: GMT
