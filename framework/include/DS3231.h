#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

class DS3231 {
public:
    DS3231(i2c_inst_t* i2c, uint8_t address = 0x68);

    bool begin();
    void setTime(uint8_t hour, uint8_t minute, uint8_t second);
    void getTime(uint8_t& hour, uint8_t& minute, uint8_t& second);

    void setDate(uint8_t day, uint8_t month, uint16_t year);
    void getDate(uint8_t& day, uint8_t& month, uint16_t& year);

    void setTimestamp(time_t t);
    time_t getTimestamp();

    float getTemperature();

    // Alarm 1 & 2: normal (date-based)
    void setAlarm1(uint8_t hour, uint8_t minute, uint8_t second, uint8_t mode);
    void setAlarm2(uint8_t hour, uint8_t minute, uint8_t mode);

    // Alarm 1 & 2: day-of-week-based
    void setAlarm1DOW(uint8_t hour, uint8_t minute, uint8_t second, uint8_t dow, uint8_t mode);
    void setAlarm2DOW(uint8_t hour, uint8_t minute, uint8_t dow, uint8_t mode);

    bool isAlarm1Triggered();
    bool isAlarm2Triggered();
    void clearAlarm1();
    void clearAlarm2();

    void enableAlarmInterrupts();
    void disableAlarmInterrupts();

    bool hasLostPower();
    void clearPowerLossFlag();

private:
    i2c_inst_t* i2c_;
    uint8_t address_;

    uint8_t bcdToDec(uint8_t val);
    uint8_t decToBcd(uint8_t val);
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);
    void readRegisters(uint8_t reg, uint8_t* buf, size_t len);
    void writeRegisters(uint8_t reg, const uint8_t* buf, size_t len);
};