/**
 * @file ds3231.cpp
 * @author Ian Archbell
 * @brief Wrapper class for the ds3231
 * @version 0.1
 * @date 2025-01-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef DS_3231_H
#define DS_3231_H
#pragma once

#include "ds3231/ds3231.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

class DS3231{

    public:

        DS3231(i2c_inst_t * i2c = i2c1, uint8_t sda_pin = 6, uint8_t scl_pin = 7, uint8_t int_pin = 4, uint8_t dev_addr = DS3231_DEVICE_ADRESS, uint8_t eeprom_addr = AT24C32_EEPROM_ADRESS_0) : 
            _i2c(i2c),
            _sda_pin(sda_pin),
            _scl_pin(scl_pin),
            _int_pin(int_pin),
            _dev_addr(dev_addr),
            _eeprom_addr(eeprom_addr)
        {
                /* Initiliaze ds3231 struct. */
            ds3231_init(&_ds3231, _i2c, _dev_addr, _eeprom_addr);

        };

        bool isEnabled();

        bool setTime(ds3231_data_t* ds3231_data);

        bool setAlarm1(ds3231_data_t* ds3231_data, uint32_t seconds, enum ALARM_1_MASKS mask, gpio_irq_callback_t ds3231_interrupt_callback);
        bool setAlarm1(ds3231_alarm_1_t* alarm, enum ALARM_1_MASKS mask, gpio_irq_callback_t ds3231_interrupt_callback);

        bool getTime(ds3231_data_t* read_data);

    private:

        bool i2c_init();
        
        ds3231_t _ds3231;
        i2c_inst_t* _i2c;  
        uint8_t _sda_pin;
        uint8_t _scl_pin;
        uint8_t _int_pin;
        uint8_t _dev_addr;
        uint8_t _eeprom_addr;

};


#endif
