/**
 * @file DS3231.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-29
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "DS3231.h"
#include "hardware/i2c.h"
#include <time.h>
#include <string.h>

DS3231::DS3231(i2c_inst_t* i2c, uint8_t address) : i2c_(i2c), address_(address) {}

bool DS3231::begin() {
    return (readRegister(0x0E) & 0x80) == 0;
}

uint8_t DS3231::bcdToDec(uint8_t val) {
    return (val / 16 * 10) + (val % 16);
}

uint8_t DS3231::decToBcd(uint8_t val) {
    return (val / 10 * 16) + (val % 10);
}

void DS3231::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t buf[] = {reg, value};
    i2c_write_blocking(i2c_, address_, buf, 2, false);
}

uint8_t DS3231::readRegister(uint8_t reg) {
    uint8_t val;
    i2c_write_blocking(i2c_, address_, &reg, 1, true);
    i2c_read_blocking(i2c_, address_, &val, 1, false);
    return val;
}

void DS3231::readRegisters(uint8_t reg, uint8_t* buf, size_t len) {
    i2c_write_blocking(i2c_, address_, &reg, 1, true);
    i2c_read_blocking(i2c_, address_, buf, len, false);
}

void DS3231::writeRegisters(uint8_t reg, const uint8_t* buf, size_t len) {
    uint8_t tmp[len + 1];
    tmp[0] = reg;
    memcpy(&tmp[1], buf, len);
    i2c_write_blocking(i2c_, address_, tmp, len + 1, false);
}

void DS3231::setTime(uint8_t hour, uint8_t minute, uint8_t second) {
    uint8_t buf[3] = {
        decToBcd(second),
        decToBcd(minute),
        decToBcd(hour)
    };
    writeRegisters(0x00, buf, 3);
}

void DS3231::getTime(uint8_t& hour, uint8_t& minute, uint8_t& second) {
    uint8_t buf[3];
    readRegisters(0x00, buf, 3);
    second = bcdToDec(buf[0]);
    minute = bcdToDec(buf[1]);
    hour = bcdToDec(buf[2] & 0x3F);
}

void DS3231::setDate(uint8_t day, uint8_t month, uint16_t year) {
    uint8_t buf[4] = {
        1,
        decToBcd(day),
        decToBcd(month),
        decToBcd(year % 100)
    };
    writeRegisters(0x03, buf, 4);
}

void DS3231::getDate(uint8_t& day, uint8_t& month, uint16_t& year) {
    uint8_t buf[4];
    readRegisters(0x03, buf, 4);
    day = bcdToDec(buf[1]);
    month = bcdToDec(buf[2] & 0x1F);
    year = 2000 + bcdToDec(buf[3]);
}

void DS3231::setTimestamp(time_t t) {
    struct tm tm;
    gmtime_r(&t, &tm);
    setTime(tm.tm_hour, tm.tm_min, tm.tm_sec);
    setDate(tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
}

time_t DS3231::getTimestamp() {
    uint8_t h, m, s, d, mon;
    uint16_t y;
    getTime(h, m, s);
    getDate(d, mon, y);

    struct tm tm = {};
    tm.tm_sec = s;
    tm.tm_min = m;
    tm.tm_hour = h;
    tm.tm_mday = d;
    tm.tm_mon = mon - 1;
    tm.tm_year = y - 1900;

    return mktime(&tm);
}

float DS3231::getTemperature() {
    uint8_t buf[2];
    readRegisters(0x11, buf, 2);
    int8_t msb = buf[0];
    uint8_t lsb = buf[1];
    return msb + ((lsb >> 6) * 0.25f);
}

void DS3231::setAlarm1(uint8_t hour, uint8_t minute, uint8_t second, uint8_t mode) {
    uint8_t buf[4] = {
        decToBcd(second) | ((mode & 0x01) ? 0x80 : 0),
        decToBcd(minute) | ((mode & 0x02) ? 0x80 : 0),
        decToBcd(hour)   | ((mode & 0x04) ? 0x80 : 0),
        0x80 | ((mode & 0x08) ? 0x80 : 0) // Date match (DY/DT = 0)
    };
    writeRegisters(0x07, buf, 4);
}

void DS3231::setAlarm2(uint8_t hour, uint8_t minute, uint8_t mode) {
    uint8_t buf[3] = {
        decToBcd(minute) | ((mode & 0x01) ? 0x80 : 0),
        decToBcd(hour)   | ((mode & 0x02) ? 0x80 : 0),
        0x80 | ((mode & 0x04) ? 0x80 : 0) // Date match (DY/DT = 0)
    };
    writeRegisters(0x0B, buf, 3);
}

void DS3231::setAlarm1DOW(uint8_t hour, uint8_t minute, uint8_t second, uint8_t dow, uint8_t mode) {
    uint8_t buf[4] = {
        decToBcd(second) | ((mode & 0x01) ? 0x80 : 0),
        decToBcd(minute) | ((mode & 0x02) ? 0x80 : 0),
        decToBcd(hour)   | ((mode & 0x04) ? 0x80 : 0),
        decToBcd(dow)    | 0x40 | ((mode & 0x08) ? 0x80 : 0) // DY/DT = 1 for DOW
    };
    writeRegisters(0x07, buf, 4);
}

void DS3231::setAlarm2DOW(uint8_t hour, uint8_t minute, uint8_t dow, uint8_t mode) {
    uint8_t buf[3] = {
        decToBcd(minute) | ((mode & 0x01) ? 0x80 : 0),
        decToBcd(hour)   | ((mode & 0x02) ? 0x80 : 0),
        decToBcd(dow)    | 0x40 | ((mode & 0x04) ? 0x80 : 0) // DY/DT = 1 for DOW
    };
    writeRegisters(0x0B, buf, 3);
}

bool DS3231::isAlarm1Triggered() {
    return readRegister(0x0F) & 0x01;
}

bool DS3231::isAlarm2Triggered() {
    return readRegister(0x0F) & 0x02;
}

void DS3231::clearAlarm1() {
    uint8_t status = readRegister(0x0F);
    writeRegister(0x0F, status & ~0x01);
}

void DS3231::clearAlarm2() {
    uint8_t status = readRegister(0x0F);
    writeRegister(0x0F, status & ~0x02);
}

void DS3231::enableAlarmInterrupts() {
    uint8_t control = readRegister(0x0E);
    control |= 0x04 | 0x03;
    writeRegister(0x0E, control);
}

void DS3231::disableAlarmInterrupts() {
    uint8_t control = readRegister(0x0E);
    control &= ~0x03;
    writeRegister(0x0E, control);
}

bool DS3231::hasLostPower() {
    return readRegister(0x0F) & 0x80;
}

void DS3231::clearPowerLossFlag() {
    uint8_t status = readRegister(0x0F);
    writeRegister(0x0F, status & ~0x80);
}