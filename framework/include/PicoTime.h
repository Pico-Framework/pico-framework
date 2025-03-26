/**
 * @file PicoTime.h
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef PICO_TIME_HPP
#define PICO_TIME_HPP

#include <ctime>
#include <string>
#include "pico/util/datetime.h"
#include "ds3231.h"

class PicoTime {
    public:
        static PicoTime& getInstance();

        void setTimeZone(const std::string& tz);
        bool setSystemTime(time_t t);
        bool setSystemTime(ds3231_data_t* dt);
        
        time_t getSystemTime();
        std::string getNowHhMmSs();

        static std::string todayHhMmSsString(tm hh_mm_ss);
        static tm todayHhMmSs(tm* hh_mm_ss);

        bool setDS3231RTC(ds3231_data_t* ds3231_data);
        bool setDS3231RTC(time_t t);

        time_t ds3231_data_to_time(const ds3231_data_t* t);

        ds3231_data_t tm_to_ds3231_data(const tm* t);
        ds3231_data_t time_t_to_ds3231_data(const time_t* t);

        void printSystemTime();
        void print(const time_t* t);
        void print(const tm* t);
        void print(const ds3231_data_t* t);

    private:
        PicoTime() = default;
        PicoTime(PicoTime&&) = default;  // Enable move constructor
        PicoTime(const PicoTime&) = delete;
        PicoTime& operator=(const PicoTime&) = delete;

        static std::string _time_zone;
};

#endif // PICO_TIME_HPP